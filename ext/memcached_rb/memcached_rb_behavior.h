#ifndef _RUBY_MEMCACHED_BEHAVIOR
#define _RUBY_MEMCACHED_BEHAVIOR

void Init_memcached_rb_behavior(void);
VALUE rb_connection_get_behavior(VALUE self, VALUE rb_behavior);
VALUE rb_connection_set_behavior(VALUE self, VALUE rb_behavior, VALUE rb_value);

#endif
