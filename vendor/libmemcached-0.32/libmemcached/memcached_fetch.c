#include "common.h"
#include "memcached_io.h"

char *memcached_fetch(memcached_st *ptr, char *key, size_t *key_length, 
                      size_t *value_length, 
                      uint32_t *flags,
                      memcached_return *error)
{
  memcached_result_st *result_buffer= &ptr->result;

  unlikely (ptr->flags & MEM_USE_UDP)
  {
    *error= MEMCACHED_NOT_SUPPORTED;
    return NULL;
  }

  result_buffer= memcached_fetch_result(ptr, result_buffer, error);

  if (result_buffer == NULL || *error != MEMCACHED_SUCCESS)
  {
    WATCHPOINT_ASSERT(result_buffer == NULL);
    *value_length= 0;
    return NULL;
  }

  *value_length= memcached_string_length(&result_buffer->value);

  if (key)
  {
    strncpy(key, result_buffer->key, result_buffer->key_length);
    *key_length= result_buffer->key_length;
  }

  if (result_buffer->flags)
    *flags= result_buffer->flags;
  else
    *flags= 0;

  return memcached_string_c_copy(&result_buffer->value);
}

memcached_result_st *memcached_fetch_result(memcached_st *ptr,
                                            memcached_result_st *result,
                                            memcached_return *error)
{
  memcached_server_st *server;

  unlikely (ptr->flags & MEM_USE_UDP)
  {
    *error= MEMCACHED_NOT_SUPPORTED;
    return NULL;
  }

  if (result == NULL)
    if ((result= memcached_result_create(ptr, NULL)) == NULL)
      return NULL;

  while ((server = memcached_io_get_readable_server(ptr)) != NULL) 
  {
    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
    *error= memcached_response(server, buffer, sizeof(buffer), result);

    if (*error == MEMCACHED_SUCCESS)
      return result;
    else if (*error == MEMCACHED_END)
      memcached_server_response_reset(server);
    else
      break;
  }

  /* We have completed reading data */
  if (result->is_allocated)
    memcached_result_free(result);
  else
    memcached_string_reset(&result->value);

  return NULL;
}

memcached_return memcached_fetch_execute(memcached_st *ptr, 
                                         memcached_execute_function *callback,
                                         void *context,
                                         unsigned int number_of_callbacks)
{
  memcached_result_st *result= &ptr->result;
  memcached_return rc= MEMCACHED_FAILURE;
  unsigned int x;

  while ((result= memcached_fetch_result(ptr, result, &rc)) != NULL) 
  {
    if (rc == MEMCACHED_SUCCESS)
    {
      for (x= 0; x < number_of_callbacks; x++)
      {
        rc= (*callback[x])(ptr, result, context);
        if (rc != MEMCACHED_SUCCESS)
          break;
      }
    }
  }
  return rc;
}
