#include "common.h"
#include "memcached_io.h"

/* 
  What happens if no servers exist?
*/
char *memcached_get(memcached_st *ptr, const char *key, 
                    size_t key_length, 
                    size_t *value_length, 
                    uint32_t *flags,
                    memcached_return *error)
{
  return memcached_get_by_key(ptr, NULL, 0, key, key_length, value_length, 
                              flags, error);
}

char *memcached_get_by_key(memcached_st *ptr, 
                           const char *master_key, 
                           size_t master_key_length, 
                           const char *key, size_t key_length,
                           size_t *value_length, 
                           uint32_t *flags,
                           memcached_return *error)
{
  char *value;
  size_t dummy_length;
  uint32_t dummy_flags;
  memcached_return dummy_error;

  /* Request the key */
  *error= memcached_mget_by_key(ptr, 
                                master_key, 
                                master_key_length, 
                                (char **)&key, &key_length, 1);

  value= memcached_fetch(ptr, NULL, NULL, 
                         value_length, flags, error);
  /* This is for historical reasons */
  if (*error == MEMCACHED_END)
    *error= MEMCACHED_NOTFOUND;

  if (value == NULL)
  {
    if (ptr->get_key_failure && *error == MEMCACHED_NOTFOUND)
    {
      memcached_return rc;

      memcached_result_reset(&ptr->result);
      rc= ptr->get_key_failure(ptr, key, key_length, &ptr->result);
      
      /* On all failure drop to returning NULL */
      if (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED)
      {
        if (rc == MEMCACHED_BUFFERED)
        {
          uint8_t latch; /* We use latch to track the state of the original socket */
          latch= memcached_behavior_get(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS);
          if (latch == 0)
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 1);

          rc= memcached_set(ptr, key, key_length, 
                            memcached_result_value(&ptr->result),
                            memcached_result_length(&ptr->result),
                            0, memcached_result_flags(&ptr->result));

          if (rc == MEMCACHED_BUFFERED && latch == 0)
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 0);
        }
        else
        {
          rc= memcached_set(ptr, key, key_length, 
                            memcached_result_value(&ptr->result),
                            memcached_result_length(&ptr->result),
                            0, memcached_result_flags(&ptr->result));
        }

        if (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED)
        {
          *error= rc;
          *value_length= memcached_result_length(&ptr->result);
          *flags= memcached_result_flags(&ptr->result);
          return memcached_string_c_copy(&ptr->result.value);
        }
      }
    }

    return NULL;
  }

  (void)memcached_fetch(ptr, NULL, NULL, 
                        &dummy_length, &dummy_flags, 
                        &dummy_error);
  WATCHPOINT_ASSERT(dummy_length == 0);

  return value;
}

memcached_return memcached_mget(memcached_st *ptr, 
                                char **keys, size_t *key_length, 
                                unsigned int number_of_keys)
{
  return memcached_mget_by_key(ptr, NULL, 0, keys, key_length, number_of_keys);
}

static memcached_return binary_mget_by_key(memcached_st *ptr,
                                           unsigned int master_server_key,
                                           char **keys, size_t *key_length,
                                           unsigned int number_of_keys);

memcached_return memcached_mget_by_key(memcached_st *ptr, 
                                       const char *master_key, 
                                       size_t master_key_length,
                                       char **keys, 
                                       size_t *key_length, 
                                       unsigned int number_of_keys)
{
  unsigned int x;
  memcached_return rc= MEMCACHED_NOTFOUND;
  char *get_command= "get ";
  uint8_t get_command_length= 4;
  unsigned int master_server_key= 0;

  LIBMEMCACHED_MEMCACHED_MGET_START();
  ptr->cursor_server= 0;

  if (number_of_keys == 0)
    return MEMCACHED_NOTFOUND;

  if (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  if ((ptr->flags & MEM_VERIFY_KEY) && (memcachd_key_test(keys, key_length, number_of_keys) == MEMCACHED_BAD_KEY_PROVIDED))
    return MEMCACHED_BAD_KEY_PROVIDED;

  if (ptr->flags & MEM_SUPPORT_CAS)
  {
    get_command= "gets ";
    get_command_length= 5;
  }

  if (master_key && master_key_length)
  {
    if ((ptr->flags & MEM_VERIFY_KEY) && (memcachd_key_test((char **)&master_key, &master_key_length, 1) == MEMCACHED_BAD_KEY_PROVIDED))
      return MEMCACHED_BAD_KEY_PROVIDED;
    master_server_key= memcached_generate_hash(ptr, master_key, master_key_length);
  }

  /* 
    Here is where we pay for the non-block API. We need to remove any data sitting
    in the queue before we start our get.

    It might be optimum to bounce the connection if count > some number.
  */
  for (x= 0; x < ptr->number_of_hosts; x++)
  {
    if (memcached_server_response_count(&ptr->hosts[x]))
    {
      char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

      if (ptr->flags & MEM_NO_BLOCK)
        (void)memcached_io_write(&ptr->hosts[x], NULL, 0, 1);

      while(memcached_server_response_count(&ptr->hosts[x]))
        (void)memcached_response(&ptr->hosts[x], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, &ptr->result);
    }
  }
  
  if (ptr->flags & MEM_BINARY_PROTOCOL)
    return binary_mget_by_key(ptr, master_server_key, keys, 
                              key_length, number_of_keys);

  /* 
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  for (x= 0; x < number_of_keys; x++)
  {
    unsigned int server_key;

    if (master_server_key)
      server_key= master_server_key;
    else
      server_key= memcached_generate_hash(ptr, keys[x], key_length[x]);

    if (memcached_server_response_count(&ptr->hosts[server_key]) == 0)
    {
      rc= memcached_connect(&ptr->hosts[server_key]);

      if (rc != MEMCACHED_SUCCESS)
        continue;

      if ((memcached_io_write(&ptr->hosts[server_key], get_command, get_command_length, 0)) == -1)
      {
        rc= MEMCACHED_SOME_ERRORS;
        continue;
      }
      WATCHPOINT_ASSERT(ptr->hosts[server_key].cursor_active == 0);
      memcached_server_response_increment(&ptr->hosts[server_key]);
      WATCHPOINT_ASSERT(ptr->hosts[server_key].cursor_active == 1);
    }

    /* Only called when we have a prefix key */
    if (ptr->prefix_key[0] != 0)
    {
      if ((memcached_io_write(&ptr->hosts[server_key], ptr->prefix_key, ptr->prefix_key_length, 0)) == -1)
      {
        memcached_server_response_reset(&ptr->hosts[server_key]);
        rc= MEMCACHED_SOME_ERRORS;
        continue;
      }
    }

    if ((memcached_io_write(&ptr->hosts[server_key], keys[x], key_length[x], 0)) == -1)
    {
      memcached_server_response_reset(&ptr->hosts[server_key]);
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }

    if ((memcached_io_write(&ptr->hosts[server_key], " ", 1, 0)) == -1)
    {
      memcached_server_response_reset(&ptr->hosts[server_key]);
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }
  }

  /*
    Should we muddle on if some servers are dead?
  */
  for (x= 0; x < ptr->number_of_hosts; x++)
  {
    if (memcached_server_response_count(&ptr->hosts[x]))
    {
      /* We need to do something about non-connnected hosts in the future */
      if ((memcached_io_write(&ptr->hosts[x], "\r\n", 2, 1)) == -1)
      {
        rc= MEMCACHED_SOME_ERRORS;
      }
    }
  }

  LIBMEMCACHED_MEMCACHED_MGET_END();
  return rc;
}

static memcached_return binary_mget_by_key(memcached_st *ptr, 
                                           unsigned int master_server_key,
                                           char **keys, size_t *key_length, 
                                           unsigned int number_of_keys)
{
  memcached_return rc= MEMCACHED_NOTFOUND;
  uint32_t x;

  int flush= number_of_keys == 1;

  /* 
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  for (x= 0; x < number_of_keys; x++) 
  {
    unsigned int server_key;

    if (master_server_key)
      server_key= master_server_key;
    else
      server_key= memcached_generate_hash(ptr, keys[x], key_length[x]);

    if (memcached_server_response_count(&ptr->hosts[server_key]) == 0) 
    {
      rc= memcached_connect(&ptr->hosts[server_key]);
      if (rc != MEMCACHED_SUCCESS) 
        continue;
    }
     
    protocol_binary_request_getk request= {.bytes= {0}};
    request.message.header.request.magic= PROTOCOL_BINARY_REQ;
    if (number_of_keys == 1)
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETK;
    else
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETKQ;

    request.message.header.request.keylen= htons((uint16_t)key_length[x]);
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.bodylen= htonl(key_length[x]);
    
    if ((memcached_io_write(&ptr->hosts[server_key], request.bytes,
                            sizeof(request.bytes), 0) == -1) ||
        (memcached_io_write(&ptr->hosts[server_key], keys[x], 
                            key_length[x], flush) == -1)) 
    {
      memcached_server_response_reset(&ptr->hosts[server_key]);
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }
    memcached_server_response_increment(&ptr->hosts[server_key]);    
  }

  if (number_of_keys > 1) 
  {
    /*
     * Send a noop command to flush the buffers
     */
    protocol_binary_request_noop request= {.bytes= {0}};
    request.message.header.request.magic= PROTOCOL_BINARY_REQ;
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_NOOP;
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    
    for (x= 0; x < ptr->number_of_hosts; x++)
      if (memcached_server_response_count(&ptr->hosts[x])) 
      {
        if (memcached_io_write(&ptr->hosts[x], NULL, 0, 1) == -1) 
        {
          memcached_server_response_reset(&ptr->hosts[x]);
          memcached_io_reset(&ptr->hosts[x]);
          rc= MEMCACHED_SOME_ERRORS;
        }

        if (memcached_io_write(&ptr->hosts[x], request.bytes, 
			       sizeof(request.bytes), 1) == -1) 
        {
          memcached_server_response_reset(&ptr->hosts[x]);
          memcached_io_reset(&ptr->hosts[x]);
          rc= MEMCACHED_SOME_ERRORS;
        }
        memcached_server_response_increment(&ptr->hosts[x]);    
      }
    }


  return rc;
}
