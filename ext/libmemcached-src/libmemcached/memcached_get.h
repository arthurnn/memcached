/*
 * Summary: Get functions for libmemcached
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef __MEMCACHED_GET_H__
#define __MEMCACHED_GET_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Public defines */
char *memcached_get(memcached_st *ptr, 
                    const char *key, size_t key_length,
                    size_t *value_length, 
                    uint32_t *flags,
                    memcached_return *error);

memcached_return memcached_mget(memcached_st *ptr, 
                                char **keys, size_t *key_length, 
                                unsigned int number_of_keys);

char *memcached_get_by_key(memcached_st *ptr, 
                           const char *master_key, size_t master_key_length, 
                           const char *key, size_t key_length, 
                           size_t *value_length, 
                           uint32_t *flags,
                           memcached_return *error);

memcached_return memcached_mget_by_key(memcached_st *ptr, 
                                       const char *master_key, size_t 
                                       master_key_length,
                                       char **keys, 
                                       size_t *key_length, 
                                       unsigned int number_of_keys);

char *memcached_fetch(memcached_st *ptr, 
                      char *key, size_t *key_length, 
                      size_t *value_length, uint32_t *flags, 
                      memcached_return *error);

memcached_result_st *memcached_fetch_result(memcached_st *ptr, 
                                            memcached_result_st *result,
                                            memcached_return *error);



#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_GET_H__ */
