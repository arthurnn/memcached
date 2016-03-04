#include <taj.h>

VALUE rb_mTaj;

static VALUE allocate(VALUE klass)
{
  memcached_st *memc;
  char *config_string = "--SERVER=localhost";

  memc = memcached(config_string, strlen(config_string));
  // memc = memcached_create(NULL); TODO: Append servers

  return Data_Wrap_Struct(klass, NULL, &memcached_free, memc);
}

static VALUE set(VALUE self, VALUE key, VALUE value)
{
  memcached_st *memc;
  Data_Get_Struct(self, memcached_st, memc);

  char *mkey = StringValuePtr(key);
  char *mvalue = StringValuePtr(value);

  memcached_return_t rc = memcached_set(memc, mkey, strlen(mkey), mvalue, strlen(mvalue), (time_t)0, (uint32_t)0);

  fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc));
  return (rc == MEMCACHED_SUCCESS);
}

static VALUE get(VALUE self, VALUE key)
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
  rb_define_method(cConnection, "set", set, 2);
  rb_define_method(cConnection, "get", get, 1);

}
