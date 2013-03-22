#include "common.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

/*
  This function is used to modify the behavior of running client.

  We quit all connections so we can reset the sockets.
*/

static void set_behavior_flag(memcached_st *ptr, memcached_flags temp_flag, uint64_t data)
{
  if (data)
    ptr->flags|= temp_flag;
  else
    ptr->flags&= ~temp_flag;
}

memcached_return memcached_behavior_set(memcached_st *ptr,
                                        memcached_behavior flag,
                                        uint64_t data)
{
  switch (flag)
  {
  case MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK:
    ptr->io_msg_watermark= (uint32_t) data;
    break;
  case MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK:
    ptr->io_bytes_watermark= (uint32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH:
    ptr->io_key_prefetch = (uint32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_SND_TIMEOUT:
    ptr->snd_timeout= (int32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_RCV_TIMEOUT:
    ptr->rcv_timeout= (int32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT:
    ptr->server_failure_limit= (uint32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_BINARY_PROTOCOL:
    if (data)
        set_behavior_flag(ptr, MEM_VERIFY_KEY, 0);
    set_behavior_flag(ptr, MEM_BINARY_PROTOCOL, data);
    break;
  case MEMCACHED_BEHAVIOR_SUPPORT_CAS:
    set_behavior_flag(ptr, MEM_SUPPORT_CAS, data);
    break;
  case MEMCACHED_BEHAVIOR_NO_BLOCK:
    set_behavior_flag(ptr, MEM_NO_BLOCK, data);
    memcached_quit(ptr);
    break;
  case MEMCACHED_BEHAVIOR_BUFFER_REQUESTS:
    set_behavior_flag(ptr, MEM_BUFFER_REQUESTS, data);
    memcached_quit(ptr);
    break;
  case MEMCACHED_BEHAVIOR_USE_UDP:
    if (ptr->number_of_hosts)
      return MEMCACHED_FAILURE;
    set_behavior_flag(ptr, MEM_USE_UDP, data);
    if (data)
      set_behavior_flag(ptr,MEM_NOREPLY,data);
    break;
  case MEMCACHED_BEHAVIOR_TCP_NODELAY:
    set_behavior_flag(ptr, MEM_TCP_NODELAY, data);
    memcached_quit(ptr);
    break;
  case MEMCACHED_BEHAVIOR_DISTRIBUTION:
    {
      ptr->distribution= (memcached_server_distribution)(data);
      if (ptr->distribution == MEMCACHED_DISTRIBUTION_RANDOM)
      {
        srandom((uint32_t) time(NULL));
      }
      run_distribution(ptr);
      break;
    }
  case MEMCACHED_BEHAVIOR_KETAMA:
    {
      if (data)
      {
        ptr->hash= MEMCACHED_HASH_MD5;
        ptr->distribution= MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA;
      }
      else
      {
        ptr->hash= 0;
        ptr->distribution= 0;
      }
      run_distribution(ptr);
      break;
    }
  case MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED:
    {
      ptr->hash= MEMCACHED_HASH_MD5;
      ptr->distribution= MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA;
      set_behavior_flag(ptr, MEM_KETAMA_WEIGHTED, data);
      run_distribution(ptr);
      break;
    }
  case MEMCACHED_BEHAVIOR_HASH:
#ifndef HAVE_HSIEH_HASH
    if ((memcached_hash)(data) == MEMCACHED_HASH_HSIEH)
      return MEMCACHED_FAILURE;
#endif
    ptr->hash= (memcached_hash)(data);
    break;
  case MEMCACHED_BEHAVIOR_KETAMA_HASH:
    ptr->hash_continuum= (memcached_hash)(data);
    run_distribution(ptr);
    break;
  case MEMCACHED_BEHAVIOR_CACHE_LOOKUPS:
    set_behavior_flag(ptr, MEM_USE_CACHE_LOOKUPS, data);
    memcached_quit(ptr);
    break;
  case MEMCACHED_BEHAVIOR_VERIFY_KEY:
    if (ptr->flags & MEM_BINARY_PROTOCOL)
        break;
    set_behavior_flag(ptr, MEM_VERIFY_KEY, data);
    break;
  case MEMCACHED_BEHAVIOR_SORT_HOSTS:
    {
      set_behavior_flag(ptr, MEM_USE_SORT_HOSTS, data);
      run_distribution(ptr);

      break;
    }
  case MEMCACHED_BEHAVIOR_POLL_TIMEOUT:
    ptr->poll_timeout= (int32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_POLL_MAX_RETRIES:
    ptr->poll_max_retries= (uint32_t) data;
    break;
  case MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT:
    ptr->connect_timeout= (int32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_RETRY_TIMEOUT:
    ptr->retry_timeout= (int32_t)data;
    break;
  case MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE:
    ptr->send_size= (int32_t)data;
    memcached_quit(ptr);
    break;
  case MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE:
    ptr->recv_size= (int32_t)data;
    memcached_quit(ptr);
    break;
  case MEMCACHED_BEHAVIOR_USER_DATA:
    return MEMCACHED_FAILURE;
  case MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY:
    set_behavior_flag(ptr, MEM_HASH_WITH_PREFIX_KEY, data);
    break;
  case MEMCACHED_BEHAVIOR_NOREPLY:
    set_behavior_flag(ptr, MEM_NOREPLY, data);
    break;
  case MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS:
    set_behavior_flag(ptr, MEM_AUTO_EJECT_HOSTS, data);
    break;
  default:
    /* Shouldn't get here */
    WATCHPOINT_ASSERT(flag);
    break;
  }

  return MEMCACHED_SUCCESS;
}

uint64_t memcached_behavior_get(memcached_st *ptr,
                                memcached_behavior flag)
{
  memcached_flags temp_flag= MEM_NO_BLOCK;

  switch (flag)
  {
  case MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK:
    return ptr->io_msg_watermark;
  case MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK:
    return ptr->io_bytes_watermark;
  case MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH:
    return ptr->io_key_prefetch;
  case MEMCACHED_BEHAVIOR_BINARY_PROTOCOL:
    temp_flag= MEM_BINARY_PROTOCOL;
    break;
  case MEMCACHED_BEHAVIOR_SUPPORT_CAS:
    temp_flag= MEM_SUPPORT_CAS;
    break;
  case MEMCACHED_BEHAVIOR_CACHE_LOOKUPS:
    temp_flag= MEM_USE_CACHE_LOOKUPS;
    break;
  case MEMCACHED_BEHAVIOR_NO_BLOCK:
    temp_flag= MEM_NO_BLOCK;
    break;
  case MEMCACHED_BEHAVIOR_BUFFER_REQUESTS:
    temp_flag= MEM_BUFFER_REQUESTS;
    break;
  case MEMCACHED_BEHAVIOR_USE_UDP:
    temp_flag= MEM_USE_UDP;
    break;
  case MEMCACHED_BEHAVIOR_TCP_NODELAY:
    temp_flag= MEM_TCP_NODELAY;
    break;
  case MEMCACHED_BEHAVIOR_VERIFY_KEY:
    temp_flag= MEM_VERIFY_KEY;
    break;
  case MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED:
    temp_flag= MEM_KETAMA_WEIGHTED;
    break;
  case MEMCACHED_BEHAVIOR_DISTRIBUTION:
    return ptr->distribution;
  case MEMCACHED_BEHAVIOR_KETAMA:
    return (ptr->distribution == MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA) ? (uint64_t) 1 : 0;
  case MEMCACHED_BEHAVIOR_HASH:
    return ptr->hash;
  case MEMCACHED_BEHAVIOR_KETAMA_HASH:
    return ptr->hash_continuum;
  case MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT:
    return ptr->server_failure_limit;
  case MEMCACHED_BEHAVIOR_SORT_HOSTS:
    temp_flag= MEM_USE_SORT_HOSTS;
    break;
  case MEMCACHED_BEHAVIOR_POLL_TIMEOUT:
    return (uint64_t)ptr->poll_timeout;
  case MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT:
    return (uint64_t)ptr->connect_timeout;
  case MEMCACHED_BEHAVIOR_RETRY_TIMEOUT:
    return (uint64_t)ptr->retry_timeout;
  case MEMCACHED_BEHAVIOR_SND_TIMEOUT:
    return (uint64_t)ptr->snd_timeout;
  case MEMCACHED_BEHAVIOR_RCV_TIMEOUT:
    return (uint64_t)ptr->rcv_timeout;
  case MEMCACHED_BEHAVIOR_POLL_MAX_RETRIES:
    return (uint64_t)ptr->poll_max_retries;
  case MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE:
    {
      int sock_size;
      socklen_t sock_length= sizeof(int);

      /* REFACTOR */
      /* We just try the first host, and if it is down we return zero */
      if ((memcached_connect(&ptr->hosts[0])) != MEMCACHED_SUCCESS)
        return 0;

      if (getsockopt(ptr->hosts[0].fd, SOL_SOCKET,
                     SO_SNDBUF, &sock_size, &sock_length))
        return 0; /* Zero means error */

      return (uint64_t) sock_size;
    }
  case MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE:
    {
      int sock_size;
      socklen_t sock_length= sizeof(int);

      /* REFACTOR */
      /* We just try the first host, and if it is down we return zero */
      if ((memcached_connect(&ptr->hosts[0])) != MEMCACHED_SUCCESS)
        return 0;

      if (getsockopt(ptr->hosts[0].fd, SOL_SOCKET,
                     SO_RCVBUF, &sock_size, &sock_length))
        return 0; /* Zero means error */

      return (uint64_t) sock_size;
    }
  case MEMCACHED_BEHAVIOR_USER_DATA:
    return MEMCACHED_FAILURE;
  case MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY:
    temp_flag= MEM_HASH_WITH_PREFIX_KEY;
    break;
  case MEMCACHED_BEHAVIOR_NOREPLY:
    temp_flag= MEM_NOREPLY;
    break;
  case MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS:
    temp_flag= MEM_AUTO_EJECT_HOSTS;
    break;
  default:
    WATCHPOINT_ASSERT(flag);
    break;
  }

  WATCHPOINT_ASSERT(temp_flag); /* Programming mistake if it gets this far */
  if (ptr->flags & temp_flag)
    return 1;
  else
    return 0;
}
