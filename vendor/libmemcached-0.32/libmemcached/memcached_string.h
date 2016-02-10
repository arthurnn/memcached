/*
 * Summary: String structure used for libmemcached.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef __MEMCACHED_STRING_H__
#define __MEMCACHED_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

struct memcached_string_st {
  memcached_st *root;
  char *end;
  char *string;
  size_t current_size;
  size_t block_size;
  bool is_allocated;
};

#define memcached_string_length(A) (size_t)((A)->end - (A)->string)
#define memcached_string_set_length(A, B) (A)->end= (A)->string + B
#define memcached_string_size(A) (A)->current_size
#define memcached_string_value(A) (A)->string

LIBMEMCACHED_API
memcached_string_st *memcached_string_create(memcached_st *ptr,
                                             memcached_string_st *string,
                                             size_t initial_size);
LIBMEMCACHED_API
memcached_return memcached_string_check(memcached_string_st *string, size_t need);
LIBMEMCACHED_API
char *memcached_string_c_copy(memcached_string_st *string);
LIBMEMCACHED_API
memcached_return memcached_string_append_character(memcached_string_st *string,
                                                   char character);
LIBMEMCACHED_API
memcached_return memcached_string_append(memcached_string_st *string,
                                         const char *value, size_t length);
LIBMEMCACHED_API
memcached_return memcached_string_reset(memcached_string_st *string);
LIBMEMCACHED_API
void memcached_string_free(memcached_string_st *string);

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_STRING_H__ */
