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
  memcached_allocated is_allocated;
  memcached_st *root;
  char key[MEMCACHED_MAX_KEY];
  size_t key_length;
  memcached_string_st value;
  uint32_t flags;
  uint64_t cas;
  time_t expiration;
  /* Add result callback function */
};

/* Result Struct */
void memcached_result_free(memcached_result_st *result);
void memcached_result_reset(memcached_result_st *ptr);
memcached_result_st *memcached_result_create(memcached_st *ptr, 
                                             memcached_result_st *result);
#define memcached_result_key_value(A) (A)->key
#define memcached_result_key_length(A) (A)->key_length
#define memcached_result_string_st(A) ((A)->value)
#ifdef FIX
#define memcached_result_value(A) memcached_string_value((A)->value)
#define memcached_result_length(A) memcached_string_length((A)->value)
#else
char *memcached_result_value(memcached_result_st *ptr);
size_t memcached_result_length(memcached_result_st *ptr);
#endif
#define memcached_result_flags(A) (A)->flags
#define memcached_result_cas(A) (A)->cas
memcached_return memcached_result_set_value(memcached_result_st *ptr, char *value, size_t length);
#define memcached_result_set_flags(A,B) (A)->flags=(B)
#define memcached_result_set_expiration(A,B) (A)->expiration=(B)

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_RESULT_H__ */
