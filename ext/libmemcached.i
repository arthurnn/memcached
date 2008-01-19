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

%apply int { uint8_t, uint16_t, uint32_t, uint64_t }
//%apply size_t* INPUT {size_t* key_length}
//%apply uint64_t* INOUT {uint64_t* value}
//%apply void* OUTPUT {void* data}
//%apply size_t* OUTPUT {size_t* value_length}
//%apply memcached_st* INPUT {memcached_st* in_ptr}
//%apply memcached_st* OUTPUT {memcached_st* out_ptr}

// %include "cpointer.i"

%include "libmemcached.h"

memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index);
%{
memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index) {
  return &(in_ptr->hosts[index]);
};
%}