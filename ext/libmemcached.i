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

%apply unsigned int { 
  uint8_t, 
  uint16_t, 
  uint32_t
  uint64_t, 
  memcached_return
};

%typemap(in) (uint32_t flags) {
  $1 = (uint32_t) NUM2ULONG($input);
};
%typemap(in) (uint64_t value) {
  $1 = (uint64_t) OFFT2NUM($input);
};

%typemap(in) (char *str, size_t len) {
 $1 = STR2CSTR($input);
 $2 = (size_t) RSTRING($input)->len;
};

%apply (char* str, size_t len) {
  (char* key, size_t key_length), 
  (char* value, size_t value_length)
};


%apply unsigned int* OUTPUT {memcached_return* error}
%apply unsigned int* OUTPUT {uint32_t* flags}
%apply size_t* OUTPUT {size_t* value_length}
%apply uint64_t* OUTPUT {uint64_t *value}

%include "libmemcached.h"

VALUE memcached_get_ruby_string(memcached_st *ptr, char *key, size_t key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_get_ruby_string(memcached_st *ptr, char *key, size_t key_length, uint32_t *flags, memcached_return *error) {
  // SWIG likes to use SWIG_FromCharPtr instead of SWIG_FromCharPtrAndSize because of the
  // retval/argout split, which means it truncates values with \0 in them.
  char* svalue;
  size_t* value_length;
  svalue = memcached_get(ptr, key, key_length, value_length, flags, error);
  return rb_str_new(svalue, *value_length);
};
%}

memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index);
%{
memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index) {
  return &(in_ptr->hosts[index]);
};
%}