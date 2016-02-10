#include "common.h"

static memcached_return memcached_flush_binary(memcached_st *ptr, 
                                               time_t expiration);
static memcached_return memcached_flush_textual(memcached_st *ptr, 
                                                time_t expiration);

memcached_return memcached_flush(memcached_st *ptr, time_t expiration)
{
  memcached_return rc;

  LIBMEMCACHED_MEMCACHED_FLUSH_START();
  if (ptr->flags & MEM_BINARY_PROTOCOL)
    rc= memcached_flush_binary(ptr, expiration);
  else
    rc= memcached_flush_textual(ptr, expiration);
  LIBMEMCACHED_MEMCACHED_FLUSH_END();
  return rc;
}

static memcached_return memcached_flush_textual(memcached_st *ptr, 
                                                time_t expiration)
{
  unsigned int x;
  size_t send_length;
  memcached_return rc;
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

  unlikely (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  for (x= 0; x < ptr->number_of_hosts; x++)
  {
    bool no_reply= (ptr->flags & MEM_NOREPLY);
    if (expiration)
      send_length= (size_t) snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, 
                                     "flush_all %llu%s\r\n",
                                     (unsigned long long)expiration, no_reply ? " noreply" : "");
    else
      send_length= (size_t) snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, 
                                     "flush_all%s\r\n", no_reply ? " noreply" : "");

    rc= memcached_do(&ptr->hosts[x], buffer, send_length, 1);

    if (rc == MEMCACHED_SUCCESS && !no_reply)
      (void)memcached_response(&ptr->hosts[x], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);
  }

  return MEMCACHED_SUCCESS;
}

static memcached_return memcached_flush_binary(memcached_st *ptr, 
                                               time_t expiration)
{
  unsigned int x;
  protocol_binary_request_flush request= {.bytes= {0}};

  unlikely (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  request.message.header.request.magic= (uint8_t)PROTOCOL_BINARY_REQ;
  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_FLUSH;
  request.message.header.request.extlen= 4;
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.bodylen= htonl(request.message.header.request.extlen);
  request.message.body.expiration= htonl((uint32_t) expiration);

  for (x= 0; x < ptr->number_of_hosts; x++)
  {
    if (ptr->flags & MEM_NOREPLY)
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_FLUSHQ;
    else
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_FLUSH;
    if (memcached_do(&ptr->hosts[x], request.bytes, 
                     sizeof(request.bytes), 1) != MEMCACHED_SUCCESS) 
    {
      memcached_io_reset(&ptr->hosts[x]);
      return MEMCACHED_WRITE_FAILURE;
    } 
  }

  for (x= 0; x < ptr->number_of_hosts; x++)
  {
    if (memcached_server_response_count(&ptr->hosts[x]) > 0)
       (void)memcached_response(&ptr->hosts[x], NULL, 0, NULL);
  }

  return MEMCACHED_SUCCESS;
}
