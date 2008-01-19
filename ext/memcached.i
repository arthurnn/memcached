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

%include "memcached.h"

// Struct creation
extern memcached_st* memcached_create(memcached_st* out_ptr);
extern memcached_st* memcached_clone(memcached_st* out_ptr, memcached_st* in_ptr);
extern void memcached_free(memcached_st* in_ptr);

// Server configuration
extern uint32_t memcached_server_count(memcached_st* in_ptr);
extern memcached_return memcached_server_add(memcached_st* in_ptr, char* hostname, uint32_t port= 11211);
extern memcached_return memcached_server_add_unix_socket(memcached_st* in_ptr, char* socket);

// Client configuration
extern unsigned long long memcached_behavior_get(memcached_st* in_ptr, memcached_behavior flag);
extern memcached_return memcached_behavior_set(memcached_st* in_ptr, memcached_behavior flag, void* data);

// Setting data
extern memcached_return memcached_set(memcached_st* in_ptr, char* key, size_t key_length, char* value, size_t value_length, time_t expiration= 0, uint32_t flags= 0);
extern memcached_return memcached_append(memcached_st* in_ptr, char* key, size_t key_length, char* value, size_t value_length, time_t expiration= 0, uint32_t flags=0);
extern memcached_return memcached_prepend(memcached_st* in_ptr, char* key, size_t key_length, char* value, size_t value_length, time_t expiration= 0, uint32_t flags=0);

// Incr and decr
extern memcached_return memcached_increment(memcached_st* in_ptr, char* key, size_t key_length, uint32_t offset, uint64_t* value);
extern memcached_return memcached_decrement(memcached_st* in_ptr, char* key, size_t key_length, uint32_t offset, uint64_t* value);

// Getting data
extern char* memcached_get(memcached_st* in_ptr, char* key, size_t key_length, size_t* value_length, uint32_t* flags, memcached_return* error);
extern memcached_return memcached_mget(memcached_st* in_ptr, char** keys, size_t* key_length, uint32_t number_of_keys);
// extern memcached_result_st*  memcached_fetch_result(memcached_st* in_ptr, memcached_result_st* result, memcached_return* error);

// Deleting data
extern memcached_return memcached_delete(memcached_st* in_ptr, char* key, size_t key_length, time_t expiration= 0);

// Misc
extern char*  memcached_strerror(memcached_st* in_ptr, memcached_return rc);
extern memcached_return* memcached_version(memcached_st* in_ptr);
