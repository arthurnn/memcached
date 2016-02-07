/*
 * Summary: Result structure used for libmemcached.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef __MEMCACHED_RESULT_H__
#define __MEMCACHED_RESULT_H__

#ifdef __cplusplus
extern "C" {
#endif

struct memcached_result_st {
  uint32_t flags;
  bool is_allocated;
  time_t expiration;
  memcached_st *root;
  size_t key_length;
  uint64_t cas;
  memcached_string_st value;
  char key[MEMCACHED_MAX_KEY];
  /* Add result callback function */
};

/* Result Struct */
LIBMEMCACHED_API
void memcached_result_free(memcached_result_st *result);
LIBMEMCACHED_API
void memcached_result_reset(memcached_result_st *ptr);
LIBMEMCACHED_API
memcached_result_st *memcached_result_create(memcached_st *ptr, 
                                             memcached_result_st *result);
#define memcached_result_key_value(A) (A)->key
#define memcached_result_key_length(A) (A)->key_length
#define memcached_result_string_st(A) ((A)->value)
#ifdef FIX
#define memcached_result_value(A) memcached_string_value((A)->value)
#define memcached_result_length(A) memcached_string_length((A)->value)
#else
LIBMEMCACHED_API
char *memcached_result_value(memcached_result_st *ptr);
LIBMEMCACHED_API
size_t memcached_result_length(memcached_result_st *ptr);
#endif
#define memcached_result_flags(A) (A)->flags
#define memcached_result_cas(A) (A)->cas
LIBMEMCACHED_API
memcached_return memcached_result_set_value(memcached_result_st *ptr, const char *value, size_t length);
#define memcached_result_set_flags(A,B) (A)->flags=(B)
#define memcached_result_set_expiration(A,B) (A)->expiration=(B)

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_RESULT_H__ */
