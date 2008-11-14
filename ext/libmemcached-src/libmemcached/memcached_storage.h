/*
 * Summary: Storage functions for libmemcached
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef __MEMCACHED_STORAGE_H__
#define __MEMCACHED_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* All of the functions for adding data to the server */
memcached_return memcached_set(memcached_st *ptr, const char *key, size_t key_length, 
                               const char *value, size_t value_length, 
                               time_t expiration,
                               uint32_t  flags);
memcached_return memcached_add(memcached_st *ptr, const char *key, size_t key_length,
                               const char *value, size_t value_length, 
                               time_t expiration,
                               uint32_t  flags);
memcached_return memcached_replace(memcached_st *ptr, const char *key, size_t key_length,
                                   const char *value, size_t value_length, 
                                   time_t expiration,
                                   uint32_t  flags);
memcached_return memcached_append(memcached_st *ptr, 
                                  const char *key, size_t key_length,
                                  const char *value, size_t value_length, 
                                  time_t expiration,
                                  uint32_t flags);
memcached_return memcached_prepend(memcached_st *ptr, 
                                   const char *key, size_t key_length,
                                   const char *value, size_t value_length, 
                                   time_t expiration,
                                   uint32_t flags);
memcached_return memcached_cas(memcached_st *ptr, 
                               const char *key, size_t key_length,
                               const char *value, size_t value_length, 
                               time_t expiration,
                               uint32_t flags,
                               uint64_t cas);

memcached_return memcached_set_by_key(memcached_st *ptr, 
                                      const char *master_key, size_t master_key_length, 
                                      const char *key, size_t key_length, 
                                      const char *value, size_t value_length, 
                                      time_t expiration,
                                      uint32_t flags);

memcached_return memcached_add_by_key(memcached_st *ptr, 
                                      const char *master_key, size_t master_key_length,
                                      const char *key, size_t key_length,
                                      const char *value, size_t value_length, 
                                      time_t expiration,
                                      uint32_t flags);

memcached_return memcached_replace_by_key(memcached_st *ptr, 
                                          const char *master_key, size_t master_key_length,
                                          const char *key, size_t key_length,
                                          const char *value, size_t value_length, 
                                          time_t expiration,
                                          uint32_t flags);

memcached_return memcached_prepend_by_key(memcached_st *ptr, 
                                          const char *master_key, size_t master_key_length,
                                          const char *key, size_t key_length,
                                          const char *value, size_t value_length, 
                                          time_t expiration,
                                          uint32_t flags);

memcached_return memcached_append_by_key(memcached_st *ptr, 
                                         const char *master_key, size_t master_key_length,
                                         const char *key, size_t key_length,
                                         const char *value, size_t value_length, 
                                         time_t expiration,
                                         uint32_t flags);

memcached_return memcached_cas_by_key(memcached_st *ptr, 
                                      const char *master_key, size_t master_key_length,
                                      const char *key, size_t key_length,
                                      const char *value, size_t value_length, 
                                      time_t expiration,
                                      uint32_t flags,
                                      uint64_t cas);

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_STORAGE_H__ */
