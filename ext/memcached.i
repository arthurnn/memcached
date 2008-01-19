%module memcached
%include "typemaps.i"
%{
#include <libmemcached/memcached.h>
%}

%include "memcached.h"

// Struct creation
extern memcached_st* memcached_create(memcached_st* OUTPUT);
extern memcached_st* memcached_clone(memcached_st* OUTPUT, memcached_st* INPUT);
extern void memcached_free(memcached_st* INPUT);

// Server configuration
extern uint32_t memcached_server_count(memcached_st* INPUT);
extern memcached_return memcached_server_add(memcached_st* INPUT, char* INPUT, uint32_t port= 11211);
extern memcached_return memcached_server_add_unix_socket(memcached_st* INPUT, char* INPUT);

// Client configuration
extern unsigned long long memcached_behavior_get(memcached_st* INPUT, memcached_behavior flag);
extern memcached_return memcached_behavior_set(memcached_st* INPUT, memcached_behavior flag, void* data);

// Setting data
extern memcached_return memcached_set(memcached_st* INPUT, char* INPUT, size_t key_length, char* INPUT, size_t value_length, time_t expiration= 0, uint32_t flags= 0);
extern memcached_return memcached_append(memcached_st* INPUT, char* INPUT, size_t key_length, char* INPUT, size_t value_length, time_t expiration= 0, uint32_t flags=0);
extern memcached_return memcached_prepend(memcached_st* INPUT, char* INPUT, size_t key_length, char* INPUT, size_t value_length, time_t expiration= 0, uint32_t flags=0);

// Incr and decr
extern memcached_return memcached_increment(memcached_st* INPUT, char* INPUT, size_t key_length, uint32_t offset, uint64_t* INPUT);
extern memcached_return memcached_decrement(memcached_st* INPUT, char* INPUT, size_t key_length, uint32_t offset, uint64_t* INPUT);

// Getting data
extern char* memcached_get(memcached_st* INPUT, char* INPUT, size_t key_length, size_t* INPUT, uint32_t* INPUT, memcached_return* OUTPUT);
extern memcached_return memcached_mget(memcached_st* INPUT, char** keys, size_t* INPUT, uint32_t number_of_keys);
extern memcached_result_st*  memcached_fetch_result(memcached_st* INPUT, memcached_result_st* OUTPUT, memcached_return* OUTPUT);

// Deleting data
extern memcached_return memcached_delete(memcached_st* INPUT, char* INPUT, size_t key_length, time_t expiration= 0);

// Misc
extern char*  memcached_strerror(memcached_st* INPUT, memcached_return rc);
extern memcached_return* memcached_version(memcached_st* INPUT);
