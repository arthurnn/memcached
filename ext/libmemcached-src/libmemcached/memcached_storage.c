/*
  Memcached library

  memcached_set()
  memcached_replace()
  memcached_add()

*/
#include "common.h"
#include "memcached_io.h"

typedef enum {
  SET_OP,
  REPLACE_OP,
  ADD_OP,
  PREPEND_OP,
  APPEND_OP,
  CAS_OP,
} memcached_storage_action;

/* Inline this */
static char *storage_op_string(memcached_storage_action verb)
{
  switch (verb)
  {
  case SET_OP:
    return "set";
  case REPLACE_OP:
    return "replace";
  case ADD_OP:
    return "add";
  case PREPEND_OP:
    return "prepend";
  case APPEND_OP:
    return "append";
  case CAS_OP:
    return "cas";
  default:
    return "tosserror"; /* This is impossible, fixes issue for compiler warning in VisualStudio */
  };

  return SET_OP;
}

static memcached_return memcached_send_binary(memcached_server_st* server, 
                                              const char *key, 
                                              size_t key_length, 
                                              const char *value, 
                                              size_t value_length, 
                                              time_t expiration,
                                              uint32_t flags,
                                              uint64_t cas,
                                              memcached_storage_action verb);

static inline memcached_return memcached_send(memcached_st *ptr, 
                                              const char *master_key, size_t master_key_length, 
                                              const char *key, size_t key_length, 
                                              const char *value, size_t value_length, 
                                              time_t expiration,
                                              uint32_t flags,
                                              uint64_t cas,
                                              memcached_storage_action verb)
{
  char to_write;
  size_t write_length;
  ssize_t sent_length;
  memcached_return rc;
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  unsigned int server_key;

  WATCHPOINT_ASSERT(!(value == NULL && value_length > 0));

  unlikely (key_length == 0)
    return MEMCACHED_NO_KEY_PROVIDED;

  unlikely (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  if ((ptr->flags & MEM_VERIFY_KEY) && (memcachd_key_test((char **)&key, &key_length, 1) == MEMCACHED_BAD_KEY_PROVIDED))
    return MEMCACHED_BAD_KEY_PROVIDED;

  server_key= memcached_generate_hash(ptr, master_key, master_key_length);

  if (ptr->flags & MEM_BINARY_PROTOCOL)
    return memcached_send_binary(&ptr->hosts[server_key], key, key_length, 
                                 value, value_length, expiration, 
                                 flags, cas, verb);

  if (cas)
    write_length= snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, 
                           "%s %s%.*s %u %llu %zu %llu\r\n", storage_op_string(verb),
                           ptr->prefix_key,
                           (int)key_length, key, flags, 
                           (unsigned long long)expiration, value_length, 
                           (unsigned long long)cas);
  else
    write_length= snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, 
                           "%s %s%.*s %u %llu %zu\r\n", storage_op_string(verb),
                           ptr->prefix_key,
                           (int)key_length, key, flags, 
                           (unsigned long long)expiration, value_length);

  if (write_length >= MEMCACHED_DEFAULT_COMMAND_SIZE)
  {
    rc= MEMCACHED_WRITE_FAILURE;
    goto error;
  }

  rc=  memcached_do(&ptr->hosts[server_key], buffer, write_length, 0);
  if (rc != MEMCACHED_SUCCESS)
    goto error;

  if ((sent_length= memcached_io_write(&ptr->hosts[server_key], value, value_length, 0)) == -1)
  {
    rc= MEMCACHED_WRITE_FAILURE;
    goto error;
  }

  if ((ptr->flags & MEM_BUFFER_REQUESTS) && verb == SET_OP)
    to_write= 0;
  else
    to_write= 1;

  if ((sent_length= memcached_io_write(&ptr->hosts[server_key], "\r\n", 2, to_write)) == -1)
  {
    rc= MEMCACHED_WRITE_FAILURE;
    goto error;
  }

  if (to_write == 0)
    return MEMCACHED_BUFFERED;

  rc= memcached_response(&ptr->hosts[server_key], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);

  if (rc == MEMCACHED_STORED)
    return MEMCACHED_SUCCESS;
  else 
    return rc;

error:
  memcached_io_reset(&ptr->hosts[server_key]);

  return rc;
}


memcached_return memcached_set(memcached_st *ptr, const char *key, size_t key_length, 
                               const char *value, size_t value_length, 
                               time_t expiration,
                               uint32_t flags)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_SET_START();
  rc= memcached_send(ptr, key, key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, SET_OP);
  LIBMEMCACHED_MEMCACHED_SET_END();
  return rc;
}

memcached_return memcached_add(memcached_st *ptr, 
                               const char *key, size_t key_length,
                               const char *value, size_t value_length, 
                               time_t expiration,
                               uint32_t flags)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_ADD_START();
  rc= memcached_send(ptr, key, key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, ADD_OP);
  LIBMEMCACHED_MEMCACHED_ADD_END();
  return rc;
}

memcached_return memcached_replace(memcached_st *ptr, 
                                   const char *key, size_t key_length,
                                   const char *value, size_t value_length, 
                                   time_t expiration,
                                   uint32_t flags)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_REPLACE_START();
  rc= memcached_send(ptr, key, key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, REPLACE_OP);
  LIBMEMCACHED_MEMCACHED_REPLACE_END();
  return rc;
}

memcached_return memcached_prepend(memcached_st *ptr, 
                                   const char *key, size_t key_length,
                                   const char *value, size_t value_length, 
                                   time_t expiration,
                                   uint32_t flags)
{
  memcached_return rc;
  rc= memcached_send(ptr, key, key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, PREPEND_OP);
  return rc;
}

memcached_return memcached_append(memcached_st *ptr, 
                                  const char *key, size_t key_length,
                                  const char *value, size_t value_length, 
                                  time_t expiration,
                                  uint32_t flags)
{
  memcached_return rc;
  rc= memcached_send(ptr, key, key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, APPEND_OP);
  return rc;
}

memcached_return memcached_cas(memcached_st *ptr, 
                               const char *key, size_t key_length,
                               const char *value, size_t value_length, 
                               time_t expiration,
                               uint32_t flags,
                               uint64_t cas)
{
  memcached_return rc;
  rc= memcached_send(ptr, key, key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, cas, CAS_OP);
  return rc;
}

memcached_return memcached_set_by_key(memcached_st *ptr, 
                                      const char *master_key __attribute__((unused)), 
                                      size_t master_key_length __attribute__((unused)), 
                                      const char *key, size_t key_length, 
                                      const char *value, size_t value_length, 
                                      time_t expiration,
                                      uint32_t flags)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_SET_START();
  rc= memcached_send(ptr, master_key, master_key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, SET_OP);
  LIBMEMCACHED_MEMCACHED_SET_END();
  return rc;
}

memcached_return memcached_add_by_key(memcached_st *ptr, 
                                      const char *master_key, size_t master_key_length,
                                      const char *key, size_t key_length,
                                      const char *value, size_t value_length, 
                                      time_t expiration,
                                      uint32_t flags)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_ADD_START();
  rc= memcached_send(ptr, master_key, master_key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, ADD_OP);
  LIBMEMCACHED_MEMCACHED_ADD_END();
  return rc;
}

memcached_return memcached_replace_by_key(memcached_st *ptr, 
                                          const char *master_key, size_t master_key_length,
                                          const char *key, size_t key_length,
                                          const char *value, size_t value_length, 
                                          time_t expiration,
                                          uint32_t flags)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_REPLACE_START();
  rc= memcached_send(ptr, master_key, master_key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, REPLACE_OP);
  LIBMEMCACHED_MEMCACHED_REPLACE_END();
  return rc;
}

memcached_return memcached_prepend_by_key(memcached_st *ptr, 
                                          const char *master_key, size_t master_key_length,
                                          const char *key, size_t key_length,
                                          const char *value, size_t value_length, 
                                          time_t expiration,
                                          uint32_t flags)
{
  memcached_return rc;
  rc= memcached_send(ptr, master_key, master_key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, PREPEND_OP);
  return rc;
}

memcached_return memcached_append_by_key(memcached_st *ptr, 
                                         const char *master_key, size_t master_key_length,
                                         const char *key, size_t key_length,
                                         const char *value, size_t value_length, 
                                         time_t expiration,
                                         uint32_t flags)
{
  memcached_return rc;
  rc= memcached_send(ptr, master_key, master_key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, 0, APPEND_OP);
  return rc;
}

memcached_return memcached_cas_by_key(memcached_st *ptr, 
                                      const char *master_key, size_t master_key_length,
                                      const char *key, size_t key_length,
                                      const char *value, size_t value_length, 
                                      time_t expiration,
                                      uint32_t flags,
                                      uint64_t cas)
{
  memcached_return rc;
  rc= memcached_send(ptr, master_key, master_key_length, 
                     key, key_length, value, value_length,
                     expiration, flags, cas, CAS_OP);
  return rc;
}

static memcached_return memcached_send_binary(memcached_server_st* server, 
                                              const char *key, 
                                              size_t key_length, 
                                              const char *value, 
                                              size_t value_length, 
                                              time_t expiration,
                                              uint32_t flags,
                                              uint64_t cas,
                                              memcached_storage_action verb)
{
  char flush;
  protocol_binary_request_set request= {.bytes= {0}};
  size_t send_length= sizeof(request.bytes);

  request.message.header.request.magic= PROTOCOL_BINARY_REQ;
  switch (verb) 
  {
  case SET_OP:
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SET;
    break;
  case ADD_OP:
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_ADD;
    break;
  case REPLACE_OP:
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_REPLACE;
    break;
  case APPEND_OP:
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_APPEND;
    break;
  case PREPEND_OP:
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_PREPEND;
    break;
  case CAS_OP:
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_REPLACE;
      break;
  }

  request.message.header.request.keylen= htons((uint16_t)key_length);
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  if (verb == APPEND_OP || verb == PREPEND_OP)
    send_length -= 8; /* append & prepend does not contain extras! */
  else 
  {
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);   
    request.message.body.expiration= htonl((uint32_t)expiration);
  }
  
  request.message.header.request.bodylen= htonl(key_length + value_length + 
						request.message.header.request.extlen);
  
  if (cas)
    request.message.header.request.cas= htonll(cas);
  
  flush= ((server->root->flags & MEM_BUFFER_REQUESTS) && verb == SET_OP) ? 0 : 1;
  /* write the header */
  if ((memcached_do(server, (const char*)request.bytes, send_length, 0) != MEMCACHED_SUCCESS) ||
      (memcached_io_write(server, key, key_length, 0) == -1) ||
      (memcached_io_write(server, value, value_length, flush) == -1)) 
  {
    memcached_io_reset(server);
    return MEMCACHED_WRITE_FAILURE;
  }

  if (flush == 0)
    return MEMCACHED_BUFFERED;
  
  return memcached_response(server, NULL, 0, NULL);   
}

