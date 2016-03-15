#ifndef _RUBY_MEMCACHED_RB
#define _RUBY_MEMCACHED_RB

#include <ruby.h>
#include <libmemcached/memcached.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define UnwrapMemcached(rb, out) Data_Get_Struct(rb, memcached_st, out);

extern VALUE rb_mMemcached;
void rb_memcached_error_check(memcached_return_t rc);

void Init_memcached_rb_behavior(void);
VALUE rb_connection_get_behavior(VALUE self, VALUE rb_behavior);
VALUE rb_connection_set_behavior(VALUE self, VALUE rb_behavior, VALUE rb_value);

#endif
