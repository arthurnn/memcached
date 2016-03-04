#include <taj.h>

VALUE rb_mTaj;

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

  // memc = memcached_create(NULL); TODO: Append servers
  const char *config_string = "--SERVER=localhost";
  ctx->memc = memcached(config_string, strlen(config_string));
  return rb_obj;
}

static VALUE
rb_connection_initialize(VALUE klass, VALUE servers)
{
  return Qnil;
}

static memcached_return_t
iterate_server_function(const memcached_st *ptr, const memcached_instance_st * server, void *context)
{
  VALUE server_list = (VALUE) context;

  VALUE r_tuple = rb_ary_new3(2,
                              rb_str_new2(memcached_server_name(server)),
                              UINT2NUM(memcached_server_port(server)));

  rb_ary_push(server_list, r_tuple);

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

}
