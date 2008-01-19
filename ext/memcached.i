%module memcached
%{
#include <libmemcached/memcached.h>
%}

%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_server_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_stat_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_string_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_result_st;

%include "typemaps.i"

%apply char* INPUT {char* value, char* key, char* hostname, char* socket}
%apply char** INPUT {char** keys}
%apply size_t* INPUT {size_t* key_length}

%apply uint64_t* INOUT {uint64_t* value}

%apply void* OUTPUT {void* data}
%apply size_t* OUTPUT {size_t* value_length}
%apply memcached_st* INPUT {memcached_st* in_ptr}
%apply memcached_st* OUTPUT {memcached_st* out_ptr}

// %include "cpointer.i"

%include "/opt/local/include/libmemcached/memcached.h"
