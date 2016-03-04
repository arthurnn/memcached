#include <taj.h>

VALUE rb_mTaj,
  Taj_Server;

ID id_ivar_hostname,
  id_ivar_port,
  id_ivar_weight;

typedef struct {
  memcached_st * memc;
} Taj_ctx;

static Taj_ctx *
get_ctx(VALUE obj)
{
  Taj_ctx* ctx;
  Data_Get_Struct(obj, Taj_ctx, ctx);
  return ctx;
}

static void
free_memc(Taj_ctx * ctx)
{
  memcached_free(ctx->memc);
}

static VALUE
allocate(VALUE klass)
{
  Taj_ctx * ctx;
  VALUE rb_obj = Data_Make_Struct(klass, Taj_ctx, NULL, free_memc, ctx);

  ctx->memc = memcached_create(NULL);
  return rb_obj;
}

static VALUE
rb_connection_initialize(VALUE self, VALUE rb_servers)
{
  Taj_ctx *ctx = get_ctx(self);
  int i;

  for (i = 0; i < RARRAY_LEN(rb_servers); ++i) {
    VALUE rb_server = rb_ary_entry(rb_servers, i);

    VALUE rb_backend = rb_ary_entry(rb_server, 0);

    ID id_tcp = rb_intern("tcp");
    ID id_socket = rb_intern("socket");
    if (id_tcp == SYM2ID(rb_backend)) {
      VALUE rb_hostname = rb_ary_entry(rb_server, 1);
      VALUE rb_port = rb_ary_entry(rb_server, 2);
      // TODO: add weight
      //VALUE rb_weight = rb_ary_entry(rb_server, 3);
      memcached_server_add (ctx->memc, StringValueCStr(rb_hostname), NUM2INT(rb_port));
    } else if (id_socket == SYM2ID(rb_backend)) {
      VALUE rb_fd = rb_ary_entry(rb_server, 1);
      memcached_server_add_unix_socket (ctx->memc, StringValueCStr(rb_fd));
    }
  }

  return Qnil;
}

static memcached_return_t
iterate_server_function(const memcached_st *ptr, const memcached_instance_st * server, void *context)
{
  VALUE server_list = (VALUE) context;

  VALUE rb_server = rb_obj_alloc(Taj_Server);

  rb_ivar_set(rb_server, id_ivar_hostname, rb_str_new2(memcached_server_name(server)));
  rb_ivar_set(rb_server, id_ivar_port, UINT2NUM(memcached_server_port(server)));
  // TODO weight

  rb_ary_push(server_list, rb_server);
  return MEMCACHED_SUCCESS;
}

static VALUE
rb_connection_servers(VALUE self)
{
  Taj_ctx *ctx = get_ctx(self);

  VALUE server_list = rb_ary_new();

  memcached_server_fn callbacks[1];
  callbacks[0] = iterate_server_function;

  memcached_server_cursor(ctx->memc, callbacks, (void *)server_list, 1);
  return server_list;
}

static VALUE
rb_connection_set(VALUE self, VALUE key, VALUE value)
{
  Taj_ctx *ctx = get_ctx(self);

  char *mkey = StringValuePtr(key);
  char *mvalue = StringValuePtr(value);

  memcached_return_t rc = memcached_set(ctx->memc, mkey, strlen(mkey), mvalue, strlen(mvalue), (time_t)0, (uint32_t)0);

  fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(ctx->memc, rc));
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_get(VALUE self, VALUE key)
{
  Taj_ctx *ctx = get_ctx(self);

  char *mkey = StringValuePtr(key);

  size_t return_value_length;
  char *response  = memcached_get(ctx->memc, mkey, strlen(mkey), &return_value_length, (time_t)0, (uint32_t)0);

  return rb_str_new2(response);
}

void Init_taj(void)
{
  rb_mTaj = rb_define_module("Taj");

  VALUE cConnection = rb_define_class_under(rb_mTaj, "Connection", rb_cObject);
  rb_define_alloc_func(cConnection, allocate);

  rb_define_method(cConnection, "initialize", rb_connection_initialize, 1);
  rb_define_method(cConnection, "servers", rb_connection_servers, 0);
  rb_define_method(cConnection, "set", rb_connection_set, 2);
  rb_define_method(cConnection, "get", rb_connection_get, 1);

  Taj_Server = rb_define_class_under(rb_mTaj, "Server", rb_cObject);

  rb_define_attr(Taj_Server, "hostname", 1, 0);
  id_ivar_hostname = rb_intern("@hostname");
  rb_define_attr(Taj_Server, "port", 1, 0);
  id_ivar_port = rb_intern("@port");
  rb_define_attr(Taj_Server, "weight", 1, 0);
  id_ivar_weight = rb_intern("@weight");
}
