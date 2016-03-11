#ifndef _RUBY_MEMCACHED_RB
#define _RUBY_MEMCACHED_RB

#include <ruby.h>

#include <libmemcached/memcached.h>
//#include <libmemcached/memcached_pool.h>

typedef struct {
  memcached_st * memc;
} memcached_ctx;

extern VALUE rb_mMemcached;

memcached_ctx * get_ctx(VALUE obj);
void handle_memcached_return(memcached_return_t rc);

#endif
