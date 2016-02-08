#include "common.h"

void memcached_set_sasl_callbacks(memcached_st *ptr,
                                  const sasl_callback_t *callbacks)
{
  ptr->sasl_callbacks= callbacks;
}

const sasl_callback_t *memcached_get_sasl_callbacks(memcached_st *ptr)
{
  return ptr->sasl_callbacks;
}

/**
 * Resolve the names for both ends of a connection
 * @param fd socket to check
 * @param laddr local address (out)
 * @param raddr remote address (out)
 * @return true on success false otherwise (errno contains more info)
 */
static bool resolve_names(int fd, char *laddr, char *raddr)
{
  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  struct sockaddr_storage saddr;
  socklen_t salen= sizeof(saddr);

  if ((getsockname(fd, (struct sockaddr *)&saddr, &salen) < 0) ||
      (getnameinfo((struct sockaddr *)&saddr, salen, host, sizeof(host),
                   port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) < 0))
     return false;

  (void)sprintf(laddr, "%s;%s", host, port);
  salen= sizeof(saddr);

  if ((getpeername(fd, (struct sockaddr *)&saddr, &salen) < 0) ||
      (getnameinfo((struct sockaddr *)&saddr, salen, host, sizeof(host),
                   port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) < 0))
    return false;

  (void)sprintf(raddr, "%s;%s", host, port);

  return true;
}

memcached_return memcached_sasl_authenticate_connection(memcached_server_st *server)
{
  memcached_return rc;

  /* SANITY CHECK: SASL can only be used with the binary protocol */
  unlikely ((server->root->flags & MEM_BINARY_PROTOCOL) == 0)
    return MEMCACHED_FAILURE;

  /* Try to get the supported mech from the server. Servers without SASL
   * support will return UNKNOWN COMMAND, so we can just treat that
   * as authenticated
   */
  protocol_binary_request_no_extras request= {
    .message.header.request= {
      .magic= PROTOCOL_BINARY_REQ,
      .opcode= PROTOCOL_BINARY_CMD_SASL_LIST_MECHS
    }
  };

  if (memcached_io_write(server, request.bytes,
			 sizeof(request.bytes), 1) != sizeof(request.bytes))
    return MEMCACHED_WRITE_FAILURE;

  memcached_server_response_increment(server);

  char mech[MEMCACHED_MAX_BUFFER];
  rc= memcached_response(server, mech, sizeof(mech), NULL);
  if (rc != MEMCACHED_SUCCESS)
  {
    if (rc == MEMCACHED_PROTOCOL_ERROR)
      rc= MEMCACHED_SUCCESS;

    return rc;
  }

  /* set ip addresses */
  char laddr[NI_MAXHOST + NI_MAXSERV];
  char raddr[NI_MAXHOST + NI_MAXSERV];

  unlikely (!resolve_names(server->fd, laddr, raddr))
  {
    server->cached_errno= errno;
    return MEMCACHED_ERRNO;
  }

  sasl_conn_t *conn;
  int ret= sasl_client_new("memcached", server->hostname, laddr, raddr,
			   server->root->sasl_callbacks, 0, &conn);
  if (ret != SASL_OK)
    return MEMCACHED_AUTH_PROBLEM;

  const char *data;
  const char *chosenmech;
  unsigned int len;
  ret= sasl_client_start(conn, mech, NULL, &data, &len, &chosenmech);

  if (ret != SASL_OK && ret != SASL_CONTINUE)
  {
    rc= MEMCACHED_AUTH_PROBLEM;
    goto end;
  }

  uint16_t keylen= (uint16_t)strlen(chosenmech);
  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SASL_AUTH;
  request.message.header.request.keylen= htons(keylen);
  request.message.header.request.bodylen= htonl(len + keylen);

  do {
    /* send the packet */
    if (memcached_io_write(server, request.bytes,
                           sizeof(request.bytes), 0) != sizeof(request.bytes) ||
        memcached_io_write(server, chosenmech, keylen, 0) != keylen ||
        memcached_io_write(server, data, len, 1) != (int)len)
    {
      rc= MEMCACHED_WRITE_FAILURE;
      goto end;
    }
    memcached_server_response_increment(server);

    /* read the response */
    rc= memcached_response(server, NULL, 0, NULL);
    if (rc != MEMCACHED_AUTH_CONTINUE)
      goto end;

    ret= sasl_client_step(conn, memcached_result_value(&server->root->result),
                          (unsigned int)memcached_result_length(&server->root->result),
                          NULL, &data, &len);

    if (ret != SASL_OK && ret != SASL_CONTINUE)
    {
      rc= MEMCACHED_AUTH_PROBLEM;
      goto end;
    }

    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SASL_STEP;
    request.message.header.request.bodylen= htonl(len + keylen);
  } while (true);

end:
  /* Release resources */
  sasl_dispose(&conn);

  return rc;
}

static int get_username(void *context, int id, const char **result,
                        unsigned int *len)
{
  if (!context || !result || (id != SASL_CB_USER && id != SASL_CB_AUTHNAME))
    return SASL_BADPARAM;

  *result= context;
  if (len)
    *len= (unsigned int)strlen(*result);

  return SASL_OK;
}

static int get_password(sasl_conn_t *conn, void *context, int id,
                        sasl_secret_t **psecret)
{
  if (!conn || ! psecret || id != SASL_CB_PASS)
    return SASL_BADPARAM;

  *psecret= context;

  return SASL_OK;
}

memcached_return memcached_set_sasl_auth_data(memcached_st *ptr,
                                              const char *username,
                                              const char *password)
{
   if (ptr == NULL || username == NULL ||
       password == NULL || ptr->sasl_callbacks != NULL)
     return MEMCACHED_FAILURE;

   sasl_callback_t *cb= ptr->call_calloc(ptr, 4, sizeof(sasl_callback_t));
   char *name= ptr->call_malloc(ptr, strlen(username) + 1);
   sasl_secret_t *secret= ptr->call_malloc(ptr, strlen(password) + 1 + sizeof(*secret))
;
   if (cb == NULL || name == NULL || secret == NULL)
   {
     ptr->call_free(ptr, cb);
     ptr->call_free(ptr, name);
     ptr->call_free(ptr, secret);
     return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
   }

   secret->len= strlen(password);
   strcpy((void*)secret->data, password);

   cb[0].id= SASL_CB_USER;
   cb[0].proc= get_username;
   cb[0].context= strcpy(name, username);
   cb[1].id= SASL_CB_AUTHNAME;
   cb[1].proc= get_username;
   cb[1].context= name;
   cb[2].id= SASL_CB_PASS;
   cb[2].proc= get_password;
   cb[2].context= secret;
   cb[3].id= SASL_CB_LIST_END;

   memcached_set_sasl_callbacks(ptr, cb);

   return MEMCACHED_SUCCESS;
}

memcached_return memcached_destroy_sasl_auth_data(memcached_st *ptr)
{
   if (ptr == NULL || ptr->sasl_callbacks == NULL)
     return MEMCACHED_FAILURE;

   ptr->call_free(ptr, ptr->sasl_callbacks[0].context);
   ptr->call_free(ptr, ptr->sasl_callbacks[2].context);
   ptr->call_free(ptr, (void*)ptr->sasl_callbacks);
   ptr->sasl_callbacks= NULL;

   return MEMCACHED_SUCCESS;
}
