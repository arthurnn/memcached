/*
  This is a partial implementation for fetching/creating memcached_server_st objects.
*/
#include "common.h"

memcached_server_st *memcached_server_create(memcached_st *memc, memcached_server_st *ptr)
{
  if (ptr == NULL)
  {
    ptr= (memcached_server_st *)calloc(1, sizeof(memcached_server_st));

    if (!ptr)
      return NULL; /*  MEMCACHED_MEMORY_ALLOCATION_FAILURE */

    ptr->is_allocated= true;
  }
  else
    memset(ptr, 0, sizeof(memcached_server_st));

  ptr->root= memc;

  return ptr;
}

memcached_server_st *memcached_server_create_with(memcached_st *memc, memcached_server_st *host,
                                                  const char *hostname, unsigned int port,
                                                  uint32_t weight, memcached_connection type)
{
  host= memcached_server_create(memc, host);

  if (host == NULL)
    return NULL;

  strncpy(host->hostname, hostname, MEMCACHED_MAX_HOST_LENGTH - 1);
  host->root= memc ? memc : NULL;
  host->port= port;
  host->weight= weight;
  host->fd= -1;
  host->type= type;
  host->read_ptr= host->read_buffer;
  if (memc)
    host->next_retry= memc->retry_timeout;
  if (type == MEMCACHED_CONNECTION_UDP)
  {
    host->write_buffer_offset= UDP_DATAGRAM_HEADER_LENGTH;
    memcached_io_init_udp_header(host, 0);
  }

  return host;
}

void memcached_server_free(memcached_server_st *ptr)
{
  memcached_quit_server(ptr, 0);
  memcached_server_error_reset(ptr);

  if (ptr->address_info)
    freeaddrinfo(ptr->address_info);

  if (ptr->is_allocated)
    ptr->root->call_free(ptr->root, ptr);
  else
    memset(ptr, 0, sizeof(memcached_server_st));
}

/*
  If we do not have a valid object to clone from, we toss an error.
*/
memcached_server_st *memcached_server_clone(memcached_server_st *clone, memcached_server_st *ptr)
{
  memcached_server_st *rv= NULL;

  /* We just do a normal create if ptr is missing */
  if (ptr == NULL)
    return NULL;

  rv = memcached_server_create_with(ptr->root, clone,
                                    ptr->hostname, ptr->port, ptr->weight,
                                    ptr->type);
  if (rv != NULL)
  {
    rv->cached_errno= ptr->cached_errno;
    if (ptr->cached_server_error) {
      size_t err_len = strlen(ptr->cached_server_error) + 1;
      rv->cached_server_error = malloc(err_len);
      strncpy(rv->cached_server_error, ptr->cached_server_error, err_len);
    }
  }

  return rv;

}

memcached_return memcached_server_cursor(memcached_st *ptr,
                                         memcached_server_function *callback,
                                         void *context,
                                         unsigned int number_of_callbacks)
{
  unsigned int y;

  for (y= 0; y < ptr->number_of_hosts; y++)
  {
    unsigned int x;

    for (x= 0; x < number_of_callbacks; x++)
    {
      unsigned int iferror;

      iferror= (*callback[x])(ptr, &ptr->hosts[y], context);

      if (iferror)
        continue;
    }
  }

  return MEMCACHED_SUCCESS;
}

memcached_server_st *memcached_server_by_key(memcached_st *ptr,  const char *key, size_t key_length, memcached_return *error)
{
  uint32_t server_key;

  *error= memcached_validate_key_length(key_length,
                                        ptr->flags & MEM_BINARY_PROTOCOL);
  unlikely (*error != MEMCACHED_SUCCESS)
    return NULL;

  unlikely (ptr->number_of_hosts == 0)
  {
    *error= MEMCACHED_NO_SERVERS;
    return NULL;
  }

  if ((ptr->flags & MEM_VERIFY_KEY) && (memcached_key_test((const char **)&key, &key_length, 1) == MEMCACHED_BAD_KEY_PROVIDED))
  {
    *error= MEMCACHED_BAD_KEY_PROVIDED;
    return NULL;
  }

  server_key= memcached_generate_hash(ptr, key, key_length);

  return &ptr->hosts[server_key];
}

const char *memcached_server_error(memcached_server_st *ptr)
{
  if (ptr->cached_server_error)
    return ptr->cached_server_error;
  else
    return NULL;
}

void memcached_server_error_reset(memcached_server_st *ptr)
{
  if (ptr->cached_server_error) {
    free(ptr->cached_server_error);
    ptr->cached_server_error = 0;
  }
}
