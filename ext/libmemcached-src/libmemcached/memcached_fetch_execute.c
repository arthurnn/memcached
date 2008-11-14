#include "common.h"

memcached_return memcached_fetch_execute(memcached_st *ptr, 
                                             memcached_execute_function *callback,
                                             void *context,
                                             unsigned int number_of_callbacks)
{
  memcached_result_st *result= &ptr->result;

  while (ptr->cursor_server < ptr->number_of_hosts)
  {
    memcached_return rc;

    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

    if (memcached_server_response_count(&ptr->hosts[ptr->cursor_server]) == 0)
    {
      ptr->cursor_server++;
      continue;
    }

    rc= memcached_response(&ptr->hosts[ptr->cursor_server], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, result);

    if (rc == MEMCACHED_END) /* END means that we move on to the next */
    {
      memcached_server_response_reset(&ptr->hosts[ptr->cursor_server]);
      ptr->cursor_server++;
      continue;
    }
    else if (rc == MEMCACHED_SUCCESS)
    {
      unsigned int x;

      for (x= 0; x < number_of_callbacks; x++)
      {
        memcached_return iferror;

        iferror= (*callback[x])(ptr, result, context);

        if (iferror != MEMCACHED_SUCCESS)
          continue;
      }
    }
  }

  return MEMCACHED_SUCCESS;
}
