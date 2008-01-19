%module libmemcached
%{
#include <libmemcached/memcached.h>
%}

%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_server_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_stat_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_string_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_result_st;

%include "typemaps.i"

// Input maps
%apply unsigned short { uint8_t };
%apply unsigned int { uint16_t };
%apply unsigned long { uint32_t flags };

%typemap(in) (char *str, size_t len) {
 $1 = STR2CSTR($input);
 $2 = (size_t) RSTRING($input)->len;
};

%apply (char *str, size_t len) {
  (char *key, size_t key_length), 
  (char *value, size_t value_length)
};

// Output maps
%apply unsigned short *OUTPUT {memcached_return *error}
%apply unsigned int *OUTPUT {uint32_t *flags}
%apply size_t *OUTPUT {size_t *value_length}
%apply unsigned long long *OUTPUT {uint64_t *value}

%include "libmemcached.h"

// Manual wrappers
VALUE memcached_get_ruby_string(memcached_st *ptr, char *key, size_t key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_get_ruby_string(memcached_st *ptr, char *key, size_t key_length, uint32_t *flags, memcached_return *error) {
  // SWIG likes to use SWIG_FromCharPtr instead of SWIG_FromCharPtrAndSize because of the
  // retval/argout split, which means it truncates values with \0 in them.
  char *svalue;
  size_t *value_length;
  svalue = memcached_get(ptr, key, key_length, value_length, flags, error);
  return rb_str_new(svalue, *value_length);
};
%}

memcached_server_st *memcached_select_server_at(memcached_st *in_ptr, int index);
%{
memcached_server_st *memcached_select_server_at(memcached_st *in_ptr, int index) {
  return &(in_ptr->hosts[index]);
};
%}