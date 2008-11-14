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
  memcached_allocated is_allocated;
  char *end;
  size_t current_size;
  size_t block_size;
  char *string;
};

#define memcached_string_length(A) (size_t)((A)->end - (A)->string)
#define memcached_string_set_length(A, B) (A)->end= (A)->string + B
#define memcached_string_size(A) (A)->current_size
#define memcached_string_value(A) (A)->string

memcached_string_st *memcached_string_create(memcached_st *ptr,
                                             memcached_string_st *string,
                                             size_t initial_size);
memcached_return memcached_string_check(memcached_string_st *string, size_t need);
char *memcached_string_c_copy(memcached_string_st *string);
memcached_return memcached_string_append_character(memcached_string_st *string,
                                                   char character);
memcached_return memcached_string_append(memcached_string_st *string,
                                         char *value, size_t length);
memcached_return memcached_string_reset(memcached_string_st *string);
void memcached_string_free(memcached_string_st *string);

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_STRING_H__ */
