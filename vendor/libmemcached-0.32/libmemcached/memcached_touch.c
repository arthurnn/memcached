#include "common.h"

memcached_return memcached_touch(memcached_st *ptr,
                                   const char *key, size_t key_length,
                                   time_t expiration)
{
  return memcached_touch_by_key(ptr, key, key_length, key, key_length, expiration);
}

memcached_return memcached_touch_by_key(memcached_st *ptr,
                                          const char *master_key, size_t master_key_length,
                                          const char *key, size_t key_length,
                                          time_t expiration)
{
  memcached_return rc;

  LIBMEMCACHED_MEMCACHED_TOUCH_START();

  unlikely (ptr->flags & MEM_USE_UDP)
    return MEMCACHED_NOT_SUPPORTED;

  unlikely ((ptr->flags & MEM_BINARY_PROTOCOL) == 0)
    return MEMCACHED_NOT_SUPPORTED;

  unlikely (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  rc= memcached_validate_key_length(key_length, 1);
  unlikely (rc != MEMCACHED_SUCCESS)
    return rc;

  rc= memcached_validate_key_length(master_key_length, 1);
  unlikely (rc != MEMCACHED_SUCCESS)
    return rc;

  uint32_t server_key= memcached_generate_hash(ptr, master_key, master_key_length);
  memcached_server_st* instance= &ptr->hosts[server_key];
  rc= memcached_connect(instance);
  if (rc != MEMCACHED_SUCCESS)
    return rc;

  protocol_binary_request_touch request= {.bytes= {0}};
  request.message.header.request.magic= PROTOCOL_BINARY_REQ;
  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_TOUCH;
  request.message.header.request.extlen= 4;
  request.message.header.request.keylen= htons((uint16_t)(key_length + ptr->prefix_key_length));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.bodylen= htonl((uint32_t)(key_length + ptr->prefix_key_length + request.message.header.request.extlen));
  request.message.body.expiration= htonl((uint32_t) expiration);

  if ((memcached_do(instance, (const char*)request.bytes, sizeof(request.bytes), 0) != MEMCACHED_SUCCESS) ||
      (memcached_io_write(instance, ptr->prefix_key, ptr->prefix_key_length, 0) == -1) ||
      (memcached_io_write(instance, key, key_length, 1) == -1)) {
    memcached_io_reset(instance);
    return MEMCACHED_WRITE_FAILURE;
  }

  rc = memcached_response(instance, NULL, 0, NULL);
  return rc;
}
