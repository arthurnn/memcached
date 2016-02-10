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


  struct libmemcached_io_vector_st vector[]=
  {
    { send_length, request.bytes },
    { strlen(ptr->prefix_key), ptr->prefix_key },
    { key_length, key }
  };

  memcached_return rc= memcached_vdo(server, vector, 3, 1);
  if (rc != MEMCACHED_SUCCESS)
  {
    memcached_io_reset(server);
    return (rc == MEMCACHED_SUCCESS) ? MEMCACHED_WRITE_FAILURE : rc;
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
  struct libmemcached_io_vector_st vector[]=
  {
    { sizeof("add ") -1, "add " },
    { strlen(ptr->prefix_key), ptr->prefix_key },
    { key_length, key },
    { sizeof(" 0") -1, " 0" },
    { sizeof(" 2678400") -1, " 2678400" },
    { sizeof(" 0") -1, " 0" },
    { 2, "\r\n" },
    { 2, "\r\n" }
  };

  memcached_return rc = memcached_vdo(server, vector, 8, 1);

  if (rc == MEMCACHED_SUCCESS)
  {
    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
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

  if (ptr->flags & MEM_NOREPLY)
  {
    size_t dummy_length;
    uint32_t dummy_flags;
    memcached_return dummy_error;

    memcached_get(ptr, key, key_length, &dummy_length, &dummy_flags, &dummy_error);
    return dummy_error;
  }

  unsigned int server_key= memcached_generate_hash(ptr, key, key_length);
  memcached_server_st *server= &ptr->hosts[server_key];

  if (ptr->flags & MEM_BINARY_PROTOCOL)
    return binary_exist(ptr, server, key, key_length);
  else
    return ascii_exist(ptr, server, key, key_length);
}