%module memcached
%include "typemaps.i"
%{
#include <libmemcached/memcached.h>
#define m_obj memcached_st*
#define m_ret memcached_return
%}

%include "memcached.h"

// Struct creation
extern m_obj memcached_create(m_obj OUTPUT);
extern m_obj memcached_clone(m_obj OUTPUT, m_obj INPUT);

// Server configuration
extern uint32_t memcached_server_count(m_obj INPUT);
extern m_ret memcached_server_add(m_obj INPUT, char* hostname, uint32_t port= 11211);
extern m_ret memcached_server_add_unix_socket(m_obj INPUT, char* socket);
extern void memcached_free(m_obj INPUT);

// Client configuration
extern unsigned long long memcached_behavior_get(m_obj INPUT, memcached_behavior flag);
extern m_ret memcached_behavior_set(m_obj INPUT, memcached_behavior flag, void* data);

// Setting data
extern m_ret memcached_set(m_obj INPUT, char* key, size_t key_length, char* value, size_t value_length, time_t expiration= 0, uint32_t flags= 0);
extern m_ret memcached_append(m_obj INPUT, char* key, size_t key_length, char* value, size_t value_length, time_t expiration= 0, uint32_t flags=0);
extern m_ret memcached_prepend(m_obj INPUT, char* key, size_t key_length, char* value, size_t value_length, time_t expiration= 0, uint32_t flags=0);

// Incr and decr
extern m_ret memcached_increment(m_obj INPUT, char* key, size_t key_length, uint32_t offset, uint64_t* value);
extern m_ret memcached_decrement(m_obj INPUT, char* key, size_t key_length, uint32_t offset, uint64_t* value);

// Getting data
extern char* memcached_get(m_obj INPUT, char* key, size_t key_length, size_t* value_length, uint32_t* flags, m_ret* error);
extern m_ret memcached_mget(m_obj INPUT, char** keys, size_t* key_length, uint32_t number_of_keys);
extern memcached_result_st*  memcached_fetch_result(m_obj INPUT, memcached_result_st* result, m_ret* error);

// Deleting data
extern m_ret memcached_delete(m_obj INPUT, char* key, size_t key_length, time_t expiration= 0);

// Misc
extern char*  memcached_strerror(m_obj INPUT, m_ret rc);
extern memcached_return* memcached_version(m_obj INPUT);
