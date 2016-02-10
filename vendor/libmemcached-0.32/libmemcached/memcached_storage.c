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
  DELETE_OP,
} memcached_storage_action;

/* Inline this */
static const char *storage_op_string(memcached_storage_action verb)
{
  switch (verb)
  {
  case SET_OP:
    return "set ";
  case DELETE_OP:
    return "delete ";
  case REPLACE_OP:
    return "replace ";
  case ADD_OP:
    return "add ";
  case PREPEND_OP:
    return "prepend ";
  case APPEND_OP:
    return "append ";
  case CAS_OP:
    return "cas ";
  default:
    return "tosserror"; /* This is impossible, fixes issue for compiler warning in VisualStudio */
  }

  /* NOTREACHED */
}

static memcached_return memcached_send_binary(memcached_st *ptr,
                                              const char *master_key,
                                              size_t master_key_length,
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

  rc= memcached_validate_key_length(key_length, ptr->flags & MEM_BINARY_PROTOCOL);
  unlikely (rc != MEMCACHED_SUCCESS)
    return rc;

  unlikely (ptr->number_of_hosts == 0)
    return MEMCACHED_NO_SERVERS;

  if ((ptr->flags & MEM_VERIFY_KEY) && (memcached_key_test((const char **)&key, &key_length, 1) == MEMCACHED_BAD_KEY_PROVIDED))
    return MEMCACHED_BAD_KEY_PROVIDED;

  if (ptr->flags & MEM_BINARY_PROTOCOL)
    return memcached_send_binary(ptr, master_key, master_key_length,
                                 key, key_length,
                                 value, value_length, expiration,
                                 flags, cas, verb);

  server_key= memcached_generate_hash(ptr, master_key, master_key_length);

  if (cas)
    write_length= (size_t) snprintf(buffer, MEMCACHED_DEFAULT_COMMAND_SIZE,
                                    "%s%s%.*s %u %llu %zu %llu%s\r\n",
                                    storage_op_string(verb),
                                    ptr->prefix_key,
                                    (int)key_length, key, flags,
                                    (unsigned long long)expiration, value_length,
                                    (unsigned long long)cas,
                                    (ptr->flags & MEM_NOREPLY) ? " noreply" : "");
  else
  {
    char *buffer_ptr= buffer;
    const char *command= storage_op_string(verb);

    /* Copy in the command, no space needed, we handle that in the command function*/
    memcpy(buffer_ptr, command, strlen(command));

    /* Copy in the key prefix, switch to the buffer_ptr */
    buffer_ptr= memcpy(buffer_ptr + strlen(command) , ptr->prefix_key, strlen(ptr->prefix_key));

    /* Copy in the key, adjust point if a key prefix was used. */
    buffer_ptr= memcpy(buffer_ptr + (ptr->prefix_key ? strlen(ptr->prefix_key) : 0),
                       key, key_length);
    buffer_ptr+= key_length;
    buffer_ptr[0]=  ' ';
    buffer_ptr++;
    write_length= (size_t)(buffer_ptr - buffer);

    if (verb == DELETE_OP) {
      if (ptr->flags & MEM_NOREPLY)
        write_length+= (size_t) snprintf(buffer_ptr, MEMCACHED_DEFAULT_COMMAND_SIZE, "noreply");
    } else {
      write_length+= (size_t) snprintf(buffer_ptr, MEMCACHED_DEFAULT_COMMAND_SIZE,
                                       "%u %llu %zu%s\r\n",
                                       flags,
                                       (unsigned long long)expiration, value_length,
                                       (ptr->flags & MEM_NOREPLY) ? " noreply" : "");
    }
  }

  if (ptr->flags & MEM_USE_UDP && ptr->flags & MEM_BUFFER_REQUESTS)
  {
    size_t cmd_size= write_length + value_length + 2;
    if (cmd_size > MAX_UDP_DATAGRAM_LENGTH - UDP_DATAGRAM_HEADER_LENGTH)
      return MEMCACHED_WRITE_FAILURE;
    if (cmd_size + ptr->hosts[server_key].write_buffer_offset > MAX_UDP_DATAGRAM_LENGTH)
      memcached_io_write(&ptr->hosts[server_key], NULL, 0, 1);
  }

  if (write_length >= MEMCACHED_DEFAULT_COMMAND_SIZE)
    return MEMCACHED_WRITE_FAILURE;

  /* Send command header */
  rc=  memcached_do(&ptr->hosts[server_key], buffer, write_length, 0);
  if (rc != MEMCACHED_SUCCESS)
    return rc;

  /* Send command body */
  if ((sent_length= memcached_io_write(&ptr->hosts[server_key], value, value_length, 0)) == -1)
    return MEMCACHED_WRITE_FAILURE;

  if ((ptr->flags & MEM_BUFFER_REQUESTS) &&
      (verb == SET_OP || verb == PREPEND_OP || verb == APPEND_OP || verb == DELETE_OP))
    to_write= 0;
  else
    to_write= 1;

  if ((sent_length= memcached_io_write(&ptr->hosts[server_key], "\r\n", 2, to_write)) == -1)
    return MEMCACHED_WRITE_FAILURE;

  if (ptr->flags & MEM_NOREPLY)
    return (to_write == 0) ? MEMCACHED_BUFFERED : MEMCACHED_SUCCESS;

  if (to_write == 0)
    return MEMCACHED_BUFFERED;

  rc= memcached_response(&ptr->hosts[server_key], buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);
  if (rc == MEMCACHED_STORED || rc == MEMCACHED_DELETED)
    return MEMCACHED_SUCCESS;

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

memcached_return memcached_delete(memcached_st *ptr, const char *key, size_t key_length,
                                  time_t expiration)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_DELETE_START();
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, "", 0,
                     expiration, 0, 0, DELETE_OP);
  LIBMEMCACHED_MEMCACHED_DELETE_END();
  return rc;
}

memcached_return memcached_delete_by_key(memcached_st *ptr,
                                         const char *master_key, size_t master_key_length,
                                         const char *key, size_t key_length,
                                         time_t expiration)
{
  memcached_return rc;
  LIBMEMCACHED_MEMCACHED_DELETE_START();
  rc= memcached_send(ptr, master_key, master_key_length,
                     key, key_length, "", 0,
                     expiration, 0, 0, DELETE_OP);
  LIBMEMCACHED_MEMCACHED_DELETE_END();
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

static inline uint8_t get_com_code(memcached_storage_action verb, bool noreply)
{
  /* 0 isn't a value we want, but GCC 4.2 seems to think ret can otherwise
   * be used uninitialized in this function. FAIL */
  uint8_t ret= 0;

  if (noreply)
    switch (verb)
    {
    case SET_OP:
      ret=PROTOCOL_BINARY_CMD_SETQ;
      break;
    case DELETE_OP:
      ret=PROTOCOL_BINARY_CMD_DELETEQ;
      break;
    case ADD_OP:
      ret=PROTOCOL_BINARY_CMD_ADDQ;
      break;
    case CAS_OP: /* FALLTHROUGH */
    case REPLACE_OP:
      ret=PROTOCOL_BINARY_CMD_REPLACEQ;
      break;
    case APPEND_OP:
      ret=PROTOCOL_BINARY_CMD_APPENDQ;
      break;
    case PREPEND_OP:
      ret=PROTOCOL_BINARY_CMD_PREPENDQ;
      break;
    default:
      WATCHPOINT_ASSERT(verb);
      break;
    }
  else
    switch (verb)
    {
    case SET_OP:
      ret=PROTOCOL_BINARY_CMD_SET;
      break;
    case DELETE_OP:
      ret=PROTOCOL_BINARY_CMD_DELETE;
      break;
    case ADD_OP:
      ret=PROTOCOL_BINARY_CMD_ADD;
      break;
    case CAS_OP: /* FALLTHROUGH */
    case REPLACE_OP:
      ret=PROTOCOL_BINARY_CMD_REPLACE;
      break;
    case APPEND_OP:
      ret=PROTOCOL_BINARY_CMD_APPEND;
      break;
    case PREPEND_OP:
      ret=PROTOCOL_BINARY_CMD_PREPEND;
      break;
    default:
      WATCHPOINT_ASSERT(verb);
      break;
    }

   return ret;
}



static memcached_return memcached_send_binary(memcached_st *ptr,
                                              const char *master_key,
                                              size_t master_key_length,
                                              const char *key,
                                              size_t key_length,
                                              const char *value,
                                              size_t value_length,
                                              time_t expiration,
                                              uint32_t flags,
                                              uint64_t cas,
                                              memcached_storage_action verb)
{
  uint8_t flush;
  protocol_binary_request_set request= {.bytes= {0}};
  size_t send_length= sizeof(request.bytes);
  uint32_t server_key= memcached_generate_hash(ptr, master_key,
                                               master_key_length);
  memcached_server_st *server= &ptr->hosts[server_key];
  bool noreply= server->root->flags & MEM_NOREPLY;

  request.message.header.request.magic= PROTOCOL_BINARY_REQ;
  request.message.header.request.opcode= get_com_code(verb, noreply);
  request.message.header.request.keylen= htons((uint16_t)(ptr->prefix_key_length + key_length));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  if (verb == APPEND_OP || verb == PREPEND_OP || verb == DELETE_OP)
    send_length -= 8; /* append, delete, and prepend do not contain extras! */
  else
  {
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);
  }

  request.message.header.request.bodylen= htonl((uint32_t) (ptr->prefix_key_length + key_length + value_length +
						request.message.header.request.extlen));

  if (cas)
    request.message.header.request.cas= htonll(cas);

  flush= (uint8_t) (((server->root->flags & MEM_BUFFER_REQUESTS) && verb == SET_OP) ? 0 : 1);

  if ((server->root->flags & MEM_USE_UDP) && !flush)
  {
    size_t cmd_size= send_length + ptr->prefix_key_length + key_length + value_length;
    if (cmd_size > MAX_UDP_DATAGRAM_LENGTH - UDP_DATAGRAM_HEADER_LENGTH)
      return MEMCACHED_WRITE_FAILURE;
    if (cmd_size + server->write_buffer_offset > MAX_UDP_DATAGRAM_LENGTH)
      memcached_io_write(server,NULL,0, 1);
  }

  /* write the header */
  if ((memcached_do(server, (const char*)request.bytes, send_length, 0) != MEMCACHED_SUCCESS) ||
      (memcached_io_write(server, ptr->prefix_key, ptr->prefix_key_length, 0) == -1) ||
      (memcached_io_write(server, key, key_length, 0) == -1) ||
      (memcached_io_write(server, value, value_length, (char) flush) == -1))
  {
    memcached_io_reset(server);
    return MEMCACHED_WRITE_FAILURE;
  }

  if (flush == 0)
    return MEMCACHED_BUFFERED;

  if (noreply)
    return MEMCACHED_SUCCESS;

  return memcached_response(server, NULL, 0, NULL);
}

