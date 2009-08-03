#include "common.h"

const char * memcached_lib_version(void) 
{
  return LIBMEMCACHED_VERSION_STRING;
}

static inline memcached_return memcached_version_binary(memcached_st *ptr);
static inline memcached_return memcached_version_textual(memcached_st *ptr);

memcached_return memcached_version(memcached_st *ptr)
{
   if (ptr->flags & MEM_USE_UDP)
    return MEMCACHED_NOT_SUPPORTED;

   if (ptr->flags & MEM_BINARY_PROTOCOL)
     return memcached_version_binary(ptr);
   else
     return memcached_version_textual(ptr);      
}

static inline memcached_return memcached_version_textual(memcached_st *ptr)
{
  unsigned int x;
  size_t send_length;
  memcached_return rc;
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  char *response_ptr;
  const char *command= "version\r\n";

  send_length= strlen(command);

  rc= MEMCACHED_SUCCESS;
  for (x= 0; x < ptr->number_of_hosts; x++)
  {
    memcached_return rrc;

    rrc= memcached_do(&ptr->hosts[x], command, send_length, 1);
    if (rrc != MEMCACHED_SUCCESS)
    {
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }

    rrc= memcached_response(&ptr->hosts[x], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);
    if (rrc != MEMCACHED_SUCCESS)
    {
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }

    /* Find the space, and then move one past it to copy version */
    response_ptr= index(buffer, ' ');
    response_ptr++;

    ptr->hosts[x].major_version= (uint8_t)strtol(response_ptr, (char **)NULL, 10);
    response_ptr= index(response_ptr, '.');
    response_ptr++;
    ptr->hosts[x].minor_version= (uint8_t)strtol(response_ptr, (char **)NULL, 10);
    response_ptr= index(response_ptr, '.');
    response_ptr++;
    ptr->hosts[x].micro_version= (uint8_t)strtol(response_ptr, (char **)NULL, 10);
  }

  return rc;
}

static inline memcached_return memcached_version_binary(memcached_st *ptr)
{
  memcached_return rc;
  unsigned int x;
  protocol_binary_request_version request= { .bytes= {0}};
  request.message.header.request.magic= PROTOCOL_BINARY_REQ;
  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_VERSION;
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;

  rc= MEMCACHED_SUCCESS;
  for (x= 0; x < ptr->number_of_hosts; x++) 
  {
    memcached_return rrc;

    rrc= memcached_do(&ptr->hosts[x], request.bytes, sizeof(request.bytes), 1);
    if (rrc != MEMCACHED_SUCCESS) 
    {
      memcached_io_reset(&ptr->hosts[x]);
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }
  }

  for (x= 0; x < ptr->number_of_hosts; x++) 
    if (memcached_server_response_count(&ptr->hosts[x]) > 0) 
    {
      memcached_return rrc;
      char buffer[32];
      char *p;

      rrc= memcached_response(&ptr->hosts[x], buffer, sizeof(buffer), NULL);
      if (rrc != MEMCACHED_SUCCESS) 
      {
        memcached_io_reset(&ptr->hosts[x]);
        rc= MEMCACHED_SOME_ERRORS;
        continue;
      }

      ptr->hosts[x].major_version= (uint8_t)strtol(buffer, &p, 10);
      ptr->hosts[x].minor_version= (uint8_t)strtol(p + 1, &p, 10);
      ptr->hosts[x].micro_version= (uint8_t)strtol(p + 1, NULL, 10);
    }

  return rc;
}
