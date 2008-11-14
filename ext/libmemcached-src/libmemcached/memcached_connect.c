#include "common.h"
#include <netdb.h>
#include <poll.h>
#include <sys/time.h>

static memcached_return set_hostinfo(memcached_server_st *server)
{
  struct addrinfo *ai;
  struct addrinfo hints;
  int e;
  char str_port[NI_MAXSERV];

  sprintf(str_port, "%u", server->port);

  memset(&hints, 0, sizeof(hints));

 // hints.ai_family= AF_INET;
  if (server->type == MEMCACHED_CONNECTION_UDP)
  {
    hints.ai_protocol= IPPROTO_UDP;
    hints.ai_socktype= SOCK_DGRAM;
  }
  else
  {
    hints.ai_socktype= SOCK_STREAM;
    hints.ai_protocol= IPPROTO_TCP;
  }

  e= getaddrinfo(server->hostname, str_port, &hints, &ai);
  if (e != 0)
  {
    WATCHPOINT_STRING(server->hostname);
    WATCHPOINT_STRING(gai_strerror(e));
    return MEMCACHED_HOST_LOOKUP_FAILURE;
  }

  if (server->address_info)
  {
    freeaddrinfo(server->address_info);
    server->address_info= NULL;
  }
  server->address_info= ai;

  return MEMCACHED_SUCCESS;
}

static memcached_return set_socket_options(memcached_server_st *ptr)
{
  if (ptr->type == MEMCACHED_CONNECTION_UDP)
    return MEMCACHED_SUCCESS;

  if (ptr->root->snd_timeout)
  {
    int error;
    struct timeval waittime;

    waittime.tv_sec= 0;
    waittime.tv_usec= ptr->root->snd_timeout;

    error= setsockopt(ptr->fd, SOL_SOCKET, SO_SNDTIMEO, 
                      &waittime, (socklen_t)sizeof(struct timeval));
    WATCHPOINT_ASSERT(error == 0);
  }

  if (ptr->root->rcv_timeout)
  {
    int error;
    struct timeval waittime;

    waittime.tv_sec= 0;
    waittime.tv_usec= ptr->root->rcv_timeout;

    error= setsockopt(ptr->fd, SOL_SOCKET, SO_RCVTIMEO, 
                      &waittime, (socklen_t)sizeof(struct timeval));
    WATCHPOINT_ASSERT(error == 0);
  }

  {
    int error;
    struct linger linger;

    linger.l_onoff= 1; 
    linger.l_linger= MEMCACHED_DEFAULT_TIMEOUT; 
    error= setsockopt(ptr->fd, SOL_SOCKET, SO_LINGER, 
                      &linger, (socklen_t)sizeof(struct linger));
    WATCHPOINT_ASSERT(error == 0);
  }

  if (ptr->root->flags & MEM_TCP_NODELAY)
  {
    int flag= 1;
    int error;

    error= setsockopt(ptr->fd, IPPROTO_TCP, TCP_NODELAY, 
                      &flag, (socklen_t)sizeof(int));
    WATCHPOINT_ASSERT(error == 0);
  }

  if (ptr->root->send_size)
  {
    int error;

    error= setsockopt(ptr->fd, SOL_SOCKET, SO_SNDBUF, 
                      &ptr->root->send_size, (socklen_t)sizeof(int));
    WATCHPOINT_ASSERT(error == 0);
  }

  if (ptr->root->recv_size)
  {
    int error;

    error= setsockopt(ptr->fd, SOL_SOCKET, SO_SNDBUF, 
                      &ptr->root->recv_size, (socklen_t)sizeof(int));
    WATCHPOINT_ASSERT(error == 0);
  }

  /* For the moment, not getting a nonblocking mode will not be fatal */
  if (ptr->root->flags & MEM_NO_BLOCK)
  {
    int flags;

    flags= fcntl(ptr->fd, F_GETFL, 0);
    unlikely (flags != -1)
    {
      (void)fcntl(ptr->fd, F_SETFL, flags | O_NONBLOCK);
    }
  }

  return MEMCACHED_SUCCESS;
}

static memcached_return unix_socket_connect(memcached_server_st *ptr)
{
  struct sockaddr_un servAddr;
  socklen_t addrlen;

  if (ptr->fd == -1)
  {
    if ((ptr->fd= socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      ptr->cached_errno= errno;
      return MEMCACHED_CONNECTION_SOCKET_CREATE_FAILURE;
    }

    memset(&servAddr, 0, sizeof (struct sockaddr_un));
    servAddr.sun_family= AF_UNIX;
    strcpy(servAddr.sun_path, ptr->hostname); /* Copy filename */

    addrlen= strlen(servAddr.sun_path) + sizeof(servAddr.sun_family);

test_connect:
    if (connect(ptr->fd, 
                (struct sockaddr *)&servAddr,
                sizeof(servAddr)) < 0)
    {
      switch (errno) {
      case EINPROGRESS:
      case EALREADY:
      case EINTR:
        goto test_connect;
      case EISCONN: /* We were spinning waiting on connect */
        break;
      default:
        WATCHPOINT_ERRNO(errno);
        ptr->cached_errno= errno;
        return MEMCACHED_ERRNO;
      }
    }
  }
  return MEMCACHED_SUCCESS;
}

static memcached_return network_connect(memcached_server_st *ptr)
{
  if (ptr->fd == -1)
  {
    struct addrinfo *use;

    if (ptr->root->server_failure_limit != 0) 
    {
      if (ptr->server_failure_counter >= ptr->root->server_failure_limit) 
      {
          memcached_server_remove(ptr);
          return MEMCACHED_FAILURE;
      }
    }
    /* Old connection junk still is in the structure */
    WATCHPOINT_ASSERT(ptr->cursor_active == 0);

    if (ptr->sockaddr_inited == MEMCACHED_NOT_ALLOCATED || 
        (!(ptr->root->flags & MEM_USE_CACHE_LOOKUPS)))
    {
      memcached_return rc;

      rc= set_hostinfo(ptr);
      if (rc != MEMCACHED_SUCCESS)
        return rc;
      ptr->sockaddr_inited= MEMCACHED_ALLOCATED;
    }

    use= ptr->address_info;
    /* Create the socket */
    while (use != NULL)
    {
      if ((ptr->fd= socket(use->ai_family, 
                           use->ai_socktype, 
                           use->ai_protocol)) < 0)
      {
        ptr->cached_errno= errno;
        WATCHPOINT_ERRNO(errno);
        return MEMCACHED_CONNECTION_SOCKET_CREATE_FAILURE;
      }

      (void)set_socket_options(ptr);

      /* connect to server */
test_connect:
      if (connect(ptr->fd, 
                  use->ai_addr, 
                  use->ai_addrlen) < 0)
      {
        switch (errno) {
          /* We are spinning waiting on connect */
        case EALREADY:
        case EINPROGRESS:
          {
            struct pollfd fds[1];
            int error;

            memset(&fds, 0, sizeof(struct pollfd));
            fds[0].fd= ptr->fd;
            fds[0].events= POLLOUT |  POLLERR;
            error= poll(fds, 1, ptr->root->connect_timeout);

            if (error == 0) 
            {
              goto handle_retry;
            }
            else if (error != 1 || fds[0].revents & POLLERR)
            {
              ptr->cached_errno= errno;
              WATCHPOINT_ERRNO(ptr->cached_errno);
              WATCHPOINT_NUMBER(ptr->root->connect_timeout);
              close(ptr->fd);
              ptr->fd= -1;
              if (ptr->address_info)
              {
                freeaddrinfo(ptr->address_info);
                ptr->address_info= NULL;
              }

              if (ptr->root->retry_timeout)
              {
                struct timeval next_time;

                gettimeofday(&next_time, NULL);
                ptr->next_retry= next_time.tv_sec + ptr->root->retry_timeout;
              }
              ptr->server_failure_counter+= 1;
              return MEMCACHED_ERRNO;
            }

            break;
          }
        /* We are spinning waiting on connect */
        case EINTR:
          goto test_connect;
        case EISCONN: /* We were spinning waiting on connect */
          break;
        default:
handle_retry:
          ptr->cached_errno= errno;
          close(ptr->fd);
          ptr->fd= -1;
          if (ptr->root->retry_timeout)
          {
            struct timeval next_time;

            gettimeofday(&next_time, NULL);
            ptr->next_retry= next_time.tv_sec + ptr->root->retry_timeout;
          }
        }
      }
      else
      {
        WATCHPOINT_ASSERT(ptr->cursor_active == 0);
        ptr->server_failure_counter= 0;
        return MEMCACHED_SUCCESS;
      }
      use = use->ai_next;
    }
  }

  if (ptr->fd == -1) {
    ptr->server_failure_counter+= 1;
    return MEMCACHED_ERRNO; /* The last error should be from connect() */
  }

  ptr->server_failure_counter= 0;
  return MEMCACHED_SUCCESS; /* The last error should be from connect() */
}


memcached_return memcached_connect(memcached_server_st *ptr)
{
  memcached_return rc= MEMCACHED_NO_SERVERS;
  LIBMEMCACHED_MEMCACHED_CONNECT_START();

  if (ptr->root->retry_timeout)
  {
    struct timeval next_time;

    gettimeofday(&next_time, NULL);
    if (next_time.tv_sec < ptr->next_retry)
      return MEMCACHED_TIMEOUT;
  }
  /* We need to clean up the multi startup piece */
  switch (ptr->type)
  {
  case MEMCACHED_CONNECTION_UNKNOWN:
    WATCHPOINT_ASSERT(0);
    rc= MEMCACHED_NOT_SUPPORTED;
    break;
  case MEMCACHED_CONNECTION_UDP:
  case MEMCACHED_CONNECTION_TCP:
    rc= network_connect(ptr);
    break;
  case MEMCACHED_CONNECTION_UNIX_SOCKET:
    rc= unix_socket_connect(ptr);
    break;
  default:
    WATCHPOINT_ASSERT(0);
  }

  LIBMEMCACHED_MEMCACHED_CONNECT_END();

  return rc;
}
