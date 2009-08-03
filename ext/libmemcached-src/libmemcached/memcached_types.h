/*
 * Summary: Typpes for libmemcached
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef __MEMCACHED_TYPES_H__
#define __MEMCACHED_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct memcached_st memcached_st;
typedef struct memcached_stat_st memcached_stat_st;
typedef struct memcached_analysis_st memcached_analysis_st;
typedef struct memcached_result_st memcached_result_st;
typedef struct memcached_string_st memcached_string_st;
typedef struct memcached_server_st memcached_server_st;
typedef struct memcached_continuum_item_st memcached_continuum_item_st;
typedef memcached_return (*memcached_clone_func)(memcached_st *parent, memcached_st *clone);
typedef memcached_return (*memcached_cleanup_func)(memcached_st *ptr);
typedef void (*memcached_free_function)(memcached_st *ptr, void *mem);
typedef void *(*memcached_malloc_function)(memcached_st *ptr, const size_t size);
typedef void *(*memcached_realloc_function)(memcached_st *ptr, void *mem, const size_t size);
typedef void *(*memcached_calloc_function)(memcached_st *ptr, size_t nelem, const size_t elsize);
typedef memcached_return (*memcached_execute_function)(memcached_st *ptr, memcached_result_st *result, void *context);
typedef memcached_return (*memcached_server_function)(memcached_st *ptr, memcached_server_st *server, void *context);
typedef memcached_return (*memcached_trigger_key)(memcached_st *ptr,  
                                                  const char *key, size_t key_length, 
                                                  memcached_result_st *result);
typedef memcached_return (*memcached_trigger_delete_key)(memcached_st *ptr,  
                                                         const char *key, size_t key_length);

typedef memcached_return (*memcached_dump_func)(memcached_st *ptr,  
                                                const char *key, size_t key_length, void *context);

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_TYPES_H__ */
