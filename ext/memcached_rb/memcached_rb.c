#include <memcached_rb.h>
#include <memcached_rb_behavior.h>

VALUE rb_mMemcached,
  rb_cServer,
  rb_eMemcachedError;

ID id_ivar_hostname,
  id_ivar_port,
  id_ivar_weight,
  id_tcp,
  id_socket;

const char *MEMCACHED_ERROR_NAMES[] = {
  NULL, // MEMCACHED_SUCCESS
  "Failure", // MEMCACHED_FAILURE
  "HostLookupFailure", // MEMCACHED_HOST_LOOKUP_FAILURE
  "ConnectionFailure", // MEMCACHED_CONNECTION_FAILURE
  "ConnectionBindFailure", // MEMCACHED_CONNECTION_BIND_FAILURE
  "WriteFailure", // MEMCACHED_WRITE_FAILURE
  "ReadFailure", // MEMCACHED_READ_FAILURE
  "UnknownReadFailure", // MEMCACHED_UNKNOWN_READ_FAILURE
  "ProtocolError", // MEMCACHED_PROTOCOL_ERROR
  "ClientError", // MEMCACHED_CLIENT_ERROR
  "ServerError", // MEMCACHED_SERVER_ERROR
  "StdError", // MEMCACHED_ERROR
  "DataExist", // MEMCACHED_DATA_EXISTS
  "DataDoesNotExist", // MEMCACHED_DATA_DOES_NOT_EXIST
  NULL, // MEMCACHED_NOTSTORED
  NULL, // MEMCACHED_STORED
  NULL, // MEMCACHED_NOTFOUND
  "MemoryAllocationFailure", // MEMCACHED_MEMORY_ALLOCATION_FAILURE
  "PartialRead", // MEMCACHED_PARTIAL_READ
  "SomeError", // MEMCACHED_SOME_ERRORS
  "NoServer", // MEMCACHED_NO_SERVERS
  "End", // MEMCACHED_END
  "Deleted", // MEMCACHED_DELETED
  "Value", // MEMCACHED_VALUE
  "Stat", // MEMCACHED_STAT
  "Item", // MEMCACHED_ITEM
  "Errno", // MEMCACHED_ERRNO
  "FailUnixSocket", // MEMCACHED_FAIL_UNIX_SOCKET
  "NotSupported", // MEMCACHED_NOT_SUPPORTED
  "NoKeyProvided", // MEMCACHED_NO_KEY_PROVIDED
  "FetchNotfinished", // MEMCACHED_FETCH_NOTFINISHED
  "Timeout", // MEMCACHED_TIMEOUT
  "Buffered", // MEMCACHED_BUFFERED
  "BadKeyProvided", // MEMCACHED_BAD_KEY_PROVIDED
  "InvalidHostProtocol", // MEMCACHED_INVALID_HOST_PROTOCOL
  "ServerMarkedDead", // MEMCACHED_SERVER_MARKED_DEAD
  "UnknownStatKey", // MEMCACHED_UNKNOWN_STAT_KEY
  "E2big", // MEMCACHED_E2BIG
  "InvalidArgument", // MEMCACHED_INVALID_ARGUMENTS
  "KeyTooBig", // MEMCACHED_KEY_TOO_BIG
  "AuthProblem", // MEMCACHED_AUTH_PROBLEM
  "AuthFailure", // MEMCACHED_AUTH_FAILURE
  "AuthContinue", // MEMCACHED_AUTH_CONTINUE
  "ParseError", // MEMCACHED_PARSE_ERROR
  "ParseUserError", // MEMCACHED_PARSE_USER_ERROR
  "Deprecated", // MEMCACHED_DEPRECATED
  "InProgress", // MEMCACHED_IN_PROGRESS
  "ServerTemporarilyDisabled", // MEMCACHED_SERVER_TEMPORARILY_DISABLED
  "ServerMemoryAllocationFailure", // MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE
  "MaximumReturn", // MEMCACHED_MAXIMUM_RETURN
  "ConnectionSocketCreateFailure", // MEMCACHED_CONNECTION_SOCKET_CREATE_FAILURE
};
#define MEMCACHED_ERROR_COUNT 51

VALUE rb_eMemcachedErrors[MEMCACHED_ERROR_COUNT];

void
handle_memcached_return(memcached_return_t rc)
{
  if (rc == MEMCACHED_SUCCESS)
    return;

  VALUE rb_error, boom;
  const char * message;
  if (rc > 0 && rc < MEMCACHED_ERROR_COUNT) {
    rb_error = rb_eMemcachedErrors[rc];
    if(Qnil == rb_error)
      return;
    message = memcached_strerror(NULL, rc);
  } else {
    rb_error = rb_eMemcachedError;
    message = "Memcached returned type not handled";
  }

  boom = rb_exc_new2(rb_error, message);
  rb_exc_raise(boom);
}

memcached_ctx *
get_ctx(VALUE obj)
{
  memcached_ctx* ctx;
  Data_Get_Struct(obj, memcached_ctx, ctx);
  return ctx;
}

static void
free_memc(memcached_ctx * ctx)
{
  memcached_free(ctx->memc);
}

static VALUE
allocate(VALUE klass)
{
  memcached_ctx * ctx;
  VALUE rb_obj = Data_Make_Struct(klass, memcached_ctx, NULL, free_memc, ctx);

  ctx->memc = memcached_create(NULL);
  return rb_obj;
}

static VALUE
rb_connection_initialize(VALUE self, VALUE rb_servers)
{
  memcached_ctx *ctx = get_ctx(self);
  int i;
  memcached_return_t rc;

  for (i = 0; i < RARRAY_LEN(rb_servers); ++i) {
    VALUE rb_server = rb_ary_entry(rb_servers, i);

    VALUE rb_backend = rb_ary_entry(rb_server, 0);

    if (id_tcp == SYM2ID(rb_backend)) {
      VALUE rb_hostname = rb_ary_entry(rb_server, 1);
      VALUE rb_port = rb_ary_entry(rb_server, 2);
      // TODO: add weight
      //VALUE rb_weight = rb_ary_entry(rb_server, 3);
      rc = memcached_server_add (ctx->memc, StringValueCStr(rb_hostname), NUM2INT(rb_port));
      handle_memcached_return(rc);
    } else if (id_socket == SYM2ID(rb_backend)) {
      VALUE rb_fd = rb_ary_entry(rb_server, 1);
      rc = memcached_server_add_unix_socket (ctx->memc, StringValueCStr(rb_fd));
      handle_memcached_return(rc);
    }
  }

  // Verify keys by default
  rc = memcached_behavior_set(ctx->memc, MEMCACHED_BEHAVIOR_VERIFY_KEY, true);
  handle_memcached_return(rc);

  return Qnil;
}

static memcached_return_t
iterate_server_function(const memcached_st *ptr, const memcached_instance_st * server, void *context)
{
  VALUE server_list = (VALUE) context;

  VALUE rb_server = rb_obj_alloc(rb_cServer);

  rb_ivar_set(rb_server, id_ivar_hostname, rb_str_new2(memcached_server_name(server)));
  rb_ivar_set(rb_server, id_ivar_port, UINT2NUM(memcached_server_port(server)));
  // TODO weight

  rb_ary_push(server_list, rb_server);
  return MEMCACHED_SUCCESS;
}

static VALUE
rb_connection_servers(VALUE self)
{
  memcached_ctx *ctx = get_ctx(self);

  VALUE server_list = rb_ary_new();

  memcached_server_fn callbacks[1];
  callbacks[0] = iterate_server_function;

  memcached_server_cursor(ctx->memc, callbacks, (void *)server_list, 1);
  return server_list;
}

static VALUE
rb_connection_flush(VALUE self)
{
  memcached_ctx *ctx = get_ctx(self);

  memcached_return_t rc = memcached_flush(ctx->memc, 0);
  handle_memcached_return(rc);

  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_set(VALUE self, VALUE key, VALUE value, VALUE ttl, VALUE flags)
{
  memcached_ctx *ctx = get_ctx(self);

  char *mkey = StringValuePtr(key);
  char *mvalue = StringValuePtr(value);

  memcached_return_t rc = memcached_set(ctx->memc, mkey, strlen(mkey), mvalue, strlen(mvalue), NUM2INT(ttl), NUM2INT(flags));

  handle_memcached_return(rc);
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_get(VALUE self, VALUE key)
{
  Check_Type(key, T_STRING);

  memcached_ctx *ctx = get_ctx(self);

  char *mkey = StringValuePtr(key);

  size_t return_value_length;
  memcached_return_t rc;
  uint32_t flags; // TODO return flags so client can know if it should deserialize or decompress
  char *response  = memcached_get(ctx->memc, mkey, strlen(mkey), &return_value_length, &flags, &rc);

  handle_memcached_return(rc);
  if (rc == MEMCACHED_SUCCESS)
    return rb_str_new2(response);
  else
    return Qnil;
}

static VALUE
rb_connection_get_multi(VALUE self, VALUE rb_keys)
{
  memcached_ctx *ctx = get_ctx(self);
  VALUE rb_values = rb_hash_new();
  int i;
  long keys_len = RARRAY_LEN(rb_keys);
  const char * keys[keys_len];
  size_t key_length[keys_len];

  VALUE * arr = RARRAY_PTR(rb_keys);
  const char *key, *value;

  memcached_return_t rc;
  memcached_result_st *result;

  for(i = 0; i < keys_len; i++) {
    keys[i] = StringValuePtr(arr[i]);
    key_length[i] = strlen(keys[i]);
  }

  rc = memcached_mget(ctx->memc, keys, key_length, sizeof(keys)/sizeof(keys[0]));
  handle_memcached_return(rc);

  while ((result = memcached_fetch_result(ctx->memc, NULL, &rc))) {
    handle_memcached_return(rc);

    key = memcached_result_key_value(result);
    value = memcached_result_value(result);
    rb_hash_aset(rb_values, rb_str_new2(key), rb_str_new2(value));
    memcached_result_free(result);
  }

  return rb_values;
}

static VALUE
rb_connection_delete(VALUE self, VALUE key)
{
  memcached_ctx *ctx = get_ctx(self);
  char *mkey = StringValuePtr(key);
  memcached_return_t rc = memcached_delete(ctx->memc, mkey, strlen(mkey), (time_t)0);
  handle_memcached_return(rc);
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_add(VALUE self, VALUE rb_key, VALUE rb_value, VALUE ttl, VALUE flags)
{
  memcached_ctx *ctx = get_ctx(self);
  char *mkey = StringValuePtr(rb_key);
  char *mvalue = StringValuePtr(rb_value);

  memcached_return_t rc = memcached_add(ctx->memc, mkey, strlen(mkey), mvalue, strlen(mvalue), NUM2INT(ttl), NUM2INT(flags));
  handle_memcached_return(rc);
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_inc(VALUE self, VALUE rb_key, VALUE rb_value)
{
  memcached_ctx *ctx = get_ctx(self);
  char *mkey = StringValuePtr(rb_key);
  uint32_t offset = NUM2UINT(rb_value);
  uint64_t new_number;

  memcached_return_t rc = memcached_increment(ctx->memc, mkey, strlen(mkey), offset, &new_number);
  handle_memcached_return(rc);
  return INT2NUM(new_number);
}

static VALUE
rb_connection_dec(VALUE self, VALUE rb_key, VALUE rb_value)
{
  memcached_ctx *ctx = get_ctx(self);
  char *mkey = StringValuePtr(rb_key);
  uint32_t offset = NUM2UINT(rb_value);
  uint64_t new_number;

  memcached_return_t rc = memcached_decrement(ctx->memc, mkey, strlen(mkey), offset, &new_number);
  handle_memcached_return(rc);
  return INT2NUM(new_number);
}

static VALUE
rb_connection_exist(VALUE self, VALUE rb_key)
{
  memcached_ctx *ctx = get_ctx(self);
  char *mkey = StringValuePtr(rb_key);

  memcached_return_t rc = memcached_exist(ctx->memc, mkey, strlen(mkey));
  handle_memcached_return(rc);
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_replace(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
  memcached_ctx *ctx = get_ctx(self);

  char *mkey = StringValuePtr(rb_key);
  char *mvalue = StringValuePtr(rb_value);

  memcached_return_t rc = memcached_replace(ctx->memc, mkey, strlen(mkey), mvalue, strlen(mvalue), NUM2INT(rb_ttl), NUM2INT(rb_flags));
  handle_memcached_return(rc);
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_prepend(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
  memcached_ctx *ctx = get_ctx(self);

  char *mkey = StringValuePtr(rb_key);
  char *mvalue = StringValuePtr(rb_value);

  memcached_return_t rc = memcached_prepend(ctx->memc, mkey, strlen(mkey), mvalue, strlen(mvalue), NUM2INT(rb_ttl), NUM2INT(rb_flags));
  handle_memcached_return(rc);
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_append(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
  memcached_ctx *ctx = get_ctx(self);

  char *mkey = StringValuePtr(rb_key);
  char *mvalue = StringValuePtr(rb_value);

  memcached_return_t rc = memcached_append(ctx->memc, mkey, strlen(mkey), mvalue, strlen(mvalue), NUM2INT(rb_ttl), NUM2INT(rb_flags));
  handle_memcached_return(rc);
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_server_to_s(VALUE self)
{
  VALUE rb_ret;
  VALUE rb_hostname = rb_ivar_get(self, id_ivar_hostname);
  VALUE rb_port = rb_ivar_get(self, id_ivar_port);
  uint32_t port = NUM2UINT(rb_port);

  rb_ret = rb_str_new("", 0);
  rb_str_append(rb_ret, rb_hostname);
  if(0 != port) {
    rb_str_append(rb_ret, rb_str_new2(":"));
    rb_str_append(rb_ret, rb_fix2str(rb_port, 10));
  }

  return rb_ret;
}

void Init_memcached_rb(void)
{
  rb_mMemcached = rb_define_module("Memcached");
  int i;

  rb_eMemcachedError = rb_define_class_under(rb_mMemcached, "Error", rb_eStandardError);
  //rb_eMemcachedErrors[0] = Qnil;
  for(i = 1; i < MEMCACHED_ERROR_COUNT; i++){
    const char* klass = MEMCACHED_ERROR_NAMES[i];
    if(NULL == klass)
      rb_eMemcachedErrors[i] = Qnil;
    else
      rb_eMemcachedErrors[i] = rb_define_class_under(rb_mMemcached, klass, rb_eMemcachedError);
  }

  VALUE cConnection = rb_define_class_under(rb_mMemcached, "Connection", rb_cObject);
  rb_define_alloc_func(cConnection, allocate);

  Init_memcached_rb_behavior();

  rb_define_method(cConnection, "initialize", rb_connection_initialize, 1);
  rb_define_method(cConnection, "servers", rb_connection_servers, 0);
  rb_define_method(cConnection, "flush", rb_connection_flush, 0);
  rb_define_method(cConnection, "set", rb_connection_set, 4);
  rb_define_method(cConnection, "get", rb_connection_get, 1);
  rb_define_method(cConnection, "get_multi", rb_connection_get_multi, 1);
  rb_define_method(cConnection, "delete", rb_connection_delete, 1);
  rb_define_method(cConnection, "add", rb_connection_add, 4);
  rb_define_method(cConnection, "increment", rb_connection_inc, 2);
  rb_define_method(cConnection, "decrement", rb_connection_dec, 2);
  rb_define_method(cConnection, "exist", rb_connection_exist, 1);
  rb_define_method(cConnection, "replace", rb_connection_replace, 4);
  rb_define_method(cConnection, "prepend", rb_connection_prepend, 4);
  rb_define_method(cConnection, "append", rb_connection_append, 4);
  rb_define_method(cConnection, "get_behavior", rb_connection_get_behavior, 1);
  rb_define_method(cConnection, "set_behavior", rb_connection_set_behavior, 2);

  rb_cServer = rb_define_class_under(rb_mMemcached, "Server", rb_cObject);
  rb_define_method(rb_cServer, "to_s", rb_server_to_s, 0);

  rb_define_attr(rb_cServer, "hostname", 1, 0);
  id_ivar_hostname = rb_intern("@hostname");
  rb_define_attr(rb_cServer, "port", 1, 0);
  id_ivar_port = rb_intern("@port");
  rb_define_attr(rb_cServer, "weight", 1, 0);
  id_ivar_weight = rb_intern("@weight");

  id_tcp = rb_intern("tcp");
  id_socket = rb_intern("socket");
}
