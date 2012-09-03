#include <libmemcached/common.h>

static memcached_return binary_exist(memcached_st *ptr, memcached_server_st *server,
                                     const char* key, size_t key_length)
{
  protocol_binary_request_set request= {.bytes= {0}};
  size_t send_length= sizeof(request.bytes);

  request.message.header.request.magic= PROTOCOL_BINARY_REQ;
  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_ADD;
  request.message.header.request.keylen= htons((uint16_t)(ptr->prefix_key_length + key_length));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.extlen= 8;
  request.message.body.flags= 0;
  request.message.body.expiration= htonl(2678400);

  request.message.header.request.bodylen= htonl((uint32_t) (key_length
                                                            +ptr->prefix_key_length
                                                            +request.message.header.request.extlen));

  memcached_return rc;
  if ((rc= memcached_do(server, (const char*)request.bytes, send_length, 0)) != MEMCACHED_SUCCESS)
  {
    memcached_io_reset(server);
    return rc;
  }

  rc= memcached_response(server, NULL, 0, NULL);

  if (rc == MEMCACHED_SUCCESS)
    rc= MEMCACHED_NOTFOUND;

  if (rc == MEMCACHED_DATA_EXISTS)
    rc= MEMCACHED_SUCCESS;

  return rc;
}

static memcached_return ascii_exist(memcached_st *ptr, memcached_server_st *server,
                                     const char* key, size_t key_length)
{
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  char *buffer_ptr = buffer;

  /* Copy in the command, no space needed, we handle that in the command function*/
  memcpy(buffer_ptr, "add ", 4);

  /* Copy in the key prefix, switch to the buffer_ptr */
  buffer_ptr= memcpy(buffer_ptr + 4 , ptr->prefix_key, strlen(ptr->prefix_key));


  /* Copy in the key, adjust point if a key prefix was used. */
  buffer_ptr= memcpy(buffer_ptr + (ptr->prefix_key ? strlen(ptr->prefix_key) : 0),
                     key, key_length);
  buffer_ptr+= key_length;
  buffer_ptr[0]=  ' ';
  buffer_ptr++;

  const char *expiration= "2678400";
  size_t write_length= (size_t)(buffer_ptr - buffer);
  write_length+= (size_t) snprintf(buffer_ptr, MEMCACHED_DEFAULT_COMMAND_SIZE,
                                       "%u %llu %zu%s\r\n",
                                       0,
                                       (unsigned long long)expiration, 0,
                                       "");

  memcached_return rc=  memcached_do(server, buffer, write_length, 0);
  if (rc == MEMCACHED_SUCCESS)
  {
    rc= memcached_response(server, buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);

    if (rc == MEMCACHED_NOTSTORED)
      rc= MEMCACHED_SUCCESS;

    if (rc == MEMCACHED_STORED)
      rc= MEMCACHED_NOTFOUND;
  }

  if (rc == MEMCACHED_WRITE_FAILURE)
    memcached_io_reset(server);

  return rc;
}

memcached_return memcached_exist(memcached_st *ptr, const char *key, size_t key_length)
{
  return memcached_exist_by_key(ptr, key, key_length, key, key_length);
}

memcached_return memcached_exist_by_key(memcached_st *ptr,
                                        const char *group_key, size_t group_key_length,
                                        const char *key, size_t key_length)
{
  unlikely (ptr->flags & MEM_USE_UDP)
    return MEMCACHED_NOT_SUPPORTED;

  if (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  unsigned int server_key= memcached_generate_hash(ptr, key, key_length);
  memcached_server_st *server= &ptr->hosts[server_key];

  if (ptr->flags & MEM_BINARY_PROTOCOL)
    return binary_exist(ptr, server, key, key_length);
  else
    return ascii_exist(ptr, server, key, key_length);
}