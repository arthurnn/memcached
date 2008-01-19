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

%apply int { 
  uint8_t, 
  uint16_t, 
  uint32_t, 
  uint64_t, 
  memcached_return
};

%typemap(in) (char *str, int len) {
 $1 = STR2CSTR($input);
 $2 = (int) RSTRING($input)->len;
};
%apply (char* str, int len) {
  (char* key, size_t key_length), 
  (char* value, size_t value_length)
};

//%apply size_t* INPUT {size_t* key_length}
//%apply uint64_t* INOUT {uint64_t* value}
//%apply void* OUTPUT {void* data}
%apply unsigned int* OUTPUT {memcached_return* error}
%apply unsigned int* OUTPUT {uint32_t* flags}
%apply size_t* OUTPUT {size_t* value_length}
//%apply memcached_st* INPUT {memcached_st* in_ptr}
//%apply memcached_st* OUTPUT {memcached_st* out_ptr}

// %include "cpointer.i"
//size_t *value_length, 
//                    uint32_t *flags,
//                    memcached_return *error

%include "libmemcached.h"

memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index);
%{
memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index) {
  return &(in_ptr->hosts[index]);
};
%}