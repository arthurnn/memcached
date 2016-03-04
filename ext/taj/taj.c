#include <taj.h>

VALUE rb_mTaj;

static VALUE
allocate(VALUE klass)
{
  memcached_st *memc;
  const char *config_string = "--SERVER=localhost";

  memc = memcached(config_string, strlen(config_string));
  // memc = memcached_create(NULL); TODO: Append servers

  return Data_Wrap_Struct(klass, NULL, &memcached_free, memc);
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
  memcached_st *memc;
  Data_Get_Struct(self, memcached_st, memc);

  VALUE server_list = rb_ary_new();

  memcached_server_fn callbacks[1];
  callbacks[0] = iterate_server_function;

  memcached_server_cursor(memc, callbacks, (void *)server_list, 1);
  return server_list;
}

static VALUE
rb_connection_set(VALUE self, VALUE key, VALUE value)
{
  memcached_st *memc;
  Data_Get_Struct(self, memcached_st, memc);

  char *mkey = StringValuePtr(key);
  char *mvalue = StringValuePtr(value);

  memcached_return_t rc = memcached_set(memc, mkey, strlen(mkey), mvalue, strlen(mvalue), (time_t)0, (uint32_t)0);

  fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc));
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE
rb_connection_get(VALUE self, VALUE key)
{
  memcached_st *memc;
  Data_Get_Struct(self, memcached_st, memc);

  char *mkey = StringValuePtr(key);

  size_t return_value_length;
  char *response  = memcached_get(memc, mkey, strlen(mkey), &return_value_length, (time_t)0, (uint32_t)0);

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
