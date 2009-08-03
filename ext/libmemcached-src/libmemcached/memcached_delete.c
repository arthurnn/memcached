#include "common.h"
#include "memcached/protocol_binary.h"

memcached_return memcached_delete(memcached_st *ptr, const char *key, size_t key_length,
                                  time_t expiration)
{
  return memcached_delete_by_key(ptr, key, key_length,
                                 key, key_length, expiration);
}

static inline memcached_return binary_delete(memcached_st *ptr, 
                                             unsigned int server_key,
                                             const char *key, 
                                             size_t key_length,
					     int flush);

memcached_return memcached_delete_by_key(memcached_st *ptr, 
                                         const char *master_key, size_t master_key_length,
                                         const char *key, size_t key_length,
                                         time_t expiration)
{
  char to_write;
  size_t send_length;
  memcached_return rc;
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  unsigned int server_key;

  LIBMEMCACHED_MEMCACHED_DELETE_START();

  rc= memcached_validate_key_length(key_length, 
                                    ptr->flags & MEM_BINARY_PROTOCOL);
  unlikely (rc != MEMCACHED_SUCCESS)
    return rc;

  unlikely (ptr->hosts == NULL || ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  server_key= memcached_generate_hash(ptr, master_key, master_key_length);
  to_write= (ptr->flags & MEM_BUFFER_REQUESTS) ? 0 : 1;
  bool no_reply= (ptr->flags & MEM_NOREPLY);
     
  if (ptr->flags & MEM_BINARY_PROTOCOL) 
    rc= binary_delete(ptr, server_key, key, key_length, to_write);
  else 
  {
    if (expiration)
      send_length= snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, 
                            "delete %s%.*s %u%s\r\n",
                            ptr->prefix_key,
                            (int)key_length, key, 
                            (uint32_t)expiration, no_reply ? " noreply" :"" );
    else
       send_length= snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, 
                             "delete %s%.*s%s\r\n",
                             ptr->prefix_key,
                             (int)key_length, key, no_reply ? " noreply" :"");
    
    if (send_length >= MEMCACHED_DEFAULT_COMMAND_SIZE) 
    {
      rc= MEMCACHED_WRITE_FAILURE;
      goto error;
    }

    if (ptr->flags & MEM_USE_UDP && !to_write)
    {
      if (send_length > MAX_UDP_DATAGRAM_LENGTH - UDP_DATAGRAM_HEADER_LENGTH)
        return MEMCACHED_WRITE_FAILURE;
      if (send_length + ptr->hosts[server_key].write_buffer_offset > MAX_UDP_DATAGRAM_LENGTH)
        memcached_io_write(&ptr->hosts[server_key], NULL, 0, 1);
    }
    
    rc= memcached_do(&ptr->hosts[server_key], buffer, send_length, to_write);
  }

  if (rc != MEMCACHED_SUCCESS)
    goto error;

  if ((ptr->flags & MEM_BUFFER_REQUESTS))
    rc= MEMCACHED_BUFFERED;
  else if (!no_reply)
  {
    rc= memcached_response(&ptr->hosts[server_key], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);
    if (rc == MEMCACHED_DELETED)
      rc= MEMCACHED_SUCCESS;
  }

  if (rc == MEMCACHED_SUCCESS && ptr->delete_trigger)
    ptr->delete_trigger(ptr, key, key_length);

error:
  LIBMEMCACHED_MEMCACHED_DELETE_END();
  return rc;
}

static inline memcached_return binary_delete(memcached_st *ptr, 
                                             unsigned int server_key,
                                             const char *key, 
					     size_t key_length,
					     int flush)
{
  protocol_binary_request_delete request= {.bytes= {0}};

  request.message.header.request.magic= PROTOCOL_BINARY_REQ;
  if (ptr->flags & MEM_NOREPLY)
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_DELETEQ;
  else
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_DELETE;
  request.message.header.request.keylen= htons((uint16_t)key_length);
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.bodylen= htonl(key_length);

  if (ptr->flags & MEM_USE_UDP && !flush)
  {
    size_t cmd_size= sizeof(request.bytes) + key_length;
    if (cmd_size > MAX_UDP_DATAGRAM_LENGTH - UDP_DATAGRAM_HEADER_LENGTH)
      return MEMCACHED_WRITE_FAILURE;
    if (cmd_size + ptr->hosts[server_key].write_buffer_offset > MAX_UDP_DATAGRAM_LENGTH)
      memcached_io_write(&ptr->hosts[server_key], NULL, 0, 1);
  }
  
  memcached_return rc= MEMCACHED_SUCCESS;

  if ((memcached_do(&ptr->hosts[server_key], request.bytes, 
                    sizeof(request.bytes), 0) != MEMCACHED_SUCCESS) ||
      (memcached_io_write(&ptr->hosts[server_key], key, 
                          key_length, flush) == -1)) 
  {
    memcached_io_reset(&ptr->hosts[server_key]);
    rc= MEMCACHED_WRITE_FAILURE;
  }

  unlikely (ptr->number_of_replicas > 0) 
  {
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_DELETEQ;

    for (uint32_t x= 0; x < ptr->number_of_replicas; ++x)
    {
      ++server_key;
      if (server_key == ptr->number_of_hosts)
        server_key= 0;
  
      memcached_server_st* server= &ptr->hosts[server_key];
      if ((memcached_do(server, (const char*)request.bytes, 
                        sizeof(request.bytes), 0) != MEMCACHED_SUCCESS) ||
          (memcached_io_write(server, key, key_length, flush) == -1))
        memcached_io_reset(server);
      else
        memcached_server_response_decrement(server);
    }
  }

  return rc;
}
