#include "common.h"
#include "memcached_io.h"

memcached_return value_fetch(memcached_server_st *ptr,
                             char *buffer,
                             memcached_result_st *result)
{
  memcached_return rc= MEMCACHED_SUCCESS;
  char *string_ptr;
  char *end_ptr;
  char *next_ptr;
  size_t value_length;
  size_t read_length;
  size_t to_read;
  char *value_ptr;

  WATCHPOINT_ASSERT(ptr->root);
  end_ptr= buffer + MEMCACHED_DEFAULT_COMMAND_SIZE;

  memcached_result_reset(result);

  string_ptr= buffer;
  string_ptr+= 6; /* "VALUE " */


  /* We load the key */
  {
    char *key;
    size_t prefix_length;

    key= result->key;
    result->key_length= 0;

    for (prefix_length= ptr->root->prefix_key_length; !(iscntrl(*string_ptr) || isspace(*string_ptr)) ; string_ptr++)
    {
      if (prefix_length == 0)
      {
        *key= *string_ptr;
        key++;
        result->key_length++;
      }
      else
        prefix_length--;
    }
    result->key[result->key_length]= 0;
  }

  if (end_ptr == string_ptr)
    goto read_error;

  /* Flags fetch move past space */
  string_ptr++;
  if (end_ptr == string_ptr)
    goto read_error;
  for (next_ptr= string_ptr; isdigit(*string_ptr); string_ptr++);
  result->flags= strtoul(next_ptr, &string_ptr, 10);

  if (end_ptr == string_ptr)
    goto read_error;

  /* Length fetch move past space*/
  string_ptr++;
  if (end_ptr == string_ptr)
    goto read_error;

  for (next_ptr= string_ptr; isdigit(*string_ptr); string_ptr++);
  value_length= (size_t)strtoull(next_ptr, &string_ptr, 10);

  if (end_ptr == string_ptr)
    goto read_error;

  /* Skip spaces */
  if (*string_ptr == '\r')
  {
    /* Skip past the \r\n */
    string_ptr+= 2;
  }
  else
  {
    string_ptr++;
    for (next_ptr= string_ptr; isdigit(*string_ptr); string_ptr++);
    result->cas= strtoull(next_ptr, &string_ptr, 10);
  }

  if (end_ptr < string_ptr)
    goto read_error;

  /* We add two bytes so that we can walk the \r\n */
  rc= memcached_string_check(&result->value, value_length+2);
  if (rc != MEMCACHED_SUCCESS)
  {
    value_length= 0;
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  value_ptr= memcached_string_value(&result->value);
  read_length= 0;
  /* 
    We read the \r\n into the string since not doing so is more 
    cycles then the waster of memory to do so.

    We are null terminating through, which will most likely make
    some people lazy about using the return length.
  */
  to_read= (value_length) + 2;
  read_length= memcached_io_read(ptr, value_ptr, to_read);
  if (read_length != (size_t)(value_length + 2))
  {
    goto read_error;
  }

/* This next bit blows the API, but this is internal....*/
  {
    char *char_ptr;
    char_ptr= memcached_string_value(&result->value);;
    char_ptr[value_length]= 0;
    char_ptr[value_length + 1]= 0;
    memcached_string_set_length(&result->value, value_length);
  }

  return MEMCACHED_SUCCESS;

read_error:
  memcached_io_reset(ptr);

  return MEMCACHED_PARTIAL_READ;
}

char *memcached_fetch(memcached_st *ptr, char *key, size_t *key_length, 
                      size_t *value_length, 
                      uint32_t *flags,
                      memcached_return *error)
{
  memcached_result_st *result_buffer= &ptr->result;

  while (ptr->cursor_server < ptr->number_of_hosts)
  {
    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

    if (memcached_server_response_count(&ptr->hosts[ptr->cursor_server]) == 0)
    {
      ptr->cursor_server++;
      continue;
    }

  *error= memcached_response(&ptr->hosts[ptr->cursor_server], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, result_buffer);

    if (*error == MEMCACHED_END) /* END means that we move on to the next */
    {
      memcached_server_response_reset(&ptr->hosts[ptr->cursor_server]);
      ptr->cursor_server++;
      continue;
    }
    else if (*error == MEMCACHED_SUCCESS)
    {
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

      return  memcached_string_c_copy(&result_buffer->value);
    }
    else
    {
      *value_length= 0;
      return NULL;
    }
  }

  ptr->cursor_server= 0;
  *value_length= 0;
  return NULL;
}

memcached_result_st *memcached_fetch_result(memcached_st *ptr, 
                                            memcached_result_st *result,
                                            memcached_return *error)
{
  if (result == NULL)
    result= memcached_result_create(ptr, NULL);

  WATCHPOINT_ASSERT(result->value.is_allocated != MEMCACHED_USED);

#ifdef UNUSED
  if (ptr->flags & MEM_NO_BLOCK)
    memcached_io_preread(ptr);
#endif

  while (ptr->cursor_server < ptr->number_of_hosts)
  {
    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

    if (memcached_server_response_count(&ptr->hosts[ptr->cursor_server]) == 0)
    {
      ptr->cursor_server++;
      continue;
    }

    *error= memcached_response(&ptr->hosts[ptr->cursor_server], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, result);
    
    if (*error == MEMCACHED_END) /* END means that we move on to the next */
    {
      memcached_server_response_reset(&ptr->hosts[ptr->cursor_server]);
      ptr->cursor_server++;
      continue;
    }
    else if (*error == MEMCACHED_SUCCESS)
      return result;
    else
      return NULL;
  }

  /* We have completed reading data */
  if (result->is_allocated == MEMCACHED_ALLOCATED)
    memcached_result_free(result);
  else
    memcached_string_reset(&result->value);

  ptr->cursor_server= 0;
  return NULL;
}
