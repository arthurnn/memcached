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

%typemap(in) (char *str, size_t len) {
 $1 = STR2CSTR($input);
 $2 = (size_t) RSTRING($input)->len;
};

%apply (char* str, size_t len) {
  (char* key, size_t key_length), 
  (char* value, size_t value_length)
};

%typemap(argout) (char *INOUT, size_t *INOUT) = char *OUTPUT {
  $result = rb_str_new($1, *$2);
};
%apply (char *, size_t *) {
    (char* value, size_t *value_length)
};

%apply unsigned int* OUTPUT {memcached_return* error}
%apply unsigned int* OUTPUT {uint32_t* flags}
//%apply char* OUTPUT {char* value}
//%apply size_t* OUTPUT {size_t* value_length}

%include "cpointer.i"
%pointer_functions(memcached_return, memcached_returnp);
%pointer_functions(uint32_t, uint32_tp);
%pointer_functions(size_t, size_tp);

//size_t *value_length, 
//                    uint32_t *flags,
//                    memcached_return *error

%include "libmemcached.h"

void memcached_get_string_by_length(memcached_st *ptr, char *key, size_t key_length, char *value, size_t *value_length, uint32_t *flags, memcached_return *error);
%{
void memcached_get_string_by_length(memcached_st *ptr, char *key, size_t key_length, char *value, size_t *value_length, uint32_t *flags, memcached_return *error) {
  value = memcached_get(ptr, key, key_length, value_length, flags, error);
};
%}

memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index);
%{
memcached_server_st* memcached_select_server_at(memcached_st* in_ptr, int index) {
  return &(in_ptr->hosts[index]);
};
%}