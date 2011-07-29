/*
 * Summary: Touch function for libmemcached
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Sergey Avseyev
 */

#ifndef __MEMCACHED_TOUCH_H__
#define __MEMCACHED_TOUCH_H__

#ifdef __cplusplus
extern "C" {
#endif

LIBMEMCACHED_API
memcached_return memcached_touch(memcached_st *ptr,
                                 const char *key, size_t key_length,
                                 time_t expiration);

LIBMEMCACHED_API
memcached_return memcached_touch_by_key(memcached_st *ptr,
                                        const char *group_key, size_t group_key_length,
                                        const char *key, size_t key_length,
                                        time_t expiration);

#ifdef __cplusplus
}
#endif

#endif /* __MEMCACHED_TOUCH_H__ */
