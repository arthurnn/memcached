/*
  We use this to dump all keys.

  At this point we only support a callback method. This could be optimized by first
  calling items and finding active slabs. For the moment though we just loop through
  all slabs on servers and "grab" the keys.
*/

#include "common.h"
static memcached_return ascii_dump(memcached_st *ptr, memcached_dump_func *callback, void *context, uint32_t number_of_callbacks)
{
  memcached_return rc= 0;
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  size_t send_length;
  uint32_t server_key;
  uint32_t x;

  unlikely (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  for (server_key= 0; server_key < ptr->number_of_hosts; server_key++)
  {
    /* 256 I BELIEVE is the upper limit of slabs */
    for (x= 0; x < 256; x++)
    {
      send_length= (size_t) snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, 
                                     "stats cachedump %u 0 0\r\n", x);

      rc= memcached_do(&ptr->hosts[server_key], buffer, send_length, 1);

      unlikely (rc != MEMCACHED_SUCCESS)
        goto error;

      while (1)
      {
        uint32_t callback_counter;
        rc= memcached_response(&ptr->hosts[server_key], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);

        if (rc == MEMCACHED_ITEM)
        {
          char *string_ptr, *end_ptr;
          char *key;

          string_ptr= buffer;
          string_ptr+= 5; /* Move past ITEM */
          for (end_ptr= string_ptr; isgraph(*end_ptr); end_ptr++);
          key= string_ptr;
          key[(size_t)(end_ptr-string_ptr)]= 0;
          for (callback_counter= 0; callback_counter < number_of_callbacks; callback_counter++)
          {
            rc= (*callback[callback_counter])(ptr, key, (size_t)(end_ptr-string_ptr), context);
            if (rc != MEMCACHED_SUCCESS)
              break;
          }
        }
        else if (rc == MEMCACHED_END)
          break;
        else
          goto error;
      }
    }
  }

error:
  if (rc == MEMCACHED_END)
    return MEMCACHED_SUCCESS;
  else
    return rc;
}

memcached_return memcached_dump(memcached_st *ptr, memcached_dump_func *callback, void *context, uint32_t number_of_callbacks)
{
  /* No support for Binary protocol yet */
  if (ptr->flags & MEM_BINARY_PROTOCOL)
    return MEMCACHED_FAILURE;

  return ascii_dump(ptr, callback, context, number_of_callbacks);
}

