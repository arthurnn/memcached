#ifndef __MEMCACHED_EXIST_H__
#define __MEMCACHED_EXIST_H__

#ifdef __cplusplus
extern "C" {
#endif

LIBMEMCACHED_API
memcached_return memcached_exist(memcached_st *memc, const char *key, size_t key_length);

LIBMEMCACHED_API
memcached_return memcached_exist_by_key(memcached_st *memc,
                                          const char *group_key, size_t group_key_length,
                                          const char *key, size_t key_length);

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_EXIST_H__ */
