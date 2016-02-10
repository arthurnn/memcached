/*
 * Summary: Connection pool implementation for libmemcached.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Trond Norbye
 */

#ifndef MEMCACHED_POOL_H
#define MEMCACHED_POOL_H

#include <libmemcached/memcached.h>

#ifdef __cplusplus
extern "C" {
#endif

struct memcached_pool_st;
typedef struct memcached_pool_st memcached_pool_st;

LIBMEMCACHED_API
memcached_pool_st *memcached_pool_create(memcached_st* mmc, uint32_t initial, 
                                         uint32_t max);
LIBMEMCACHED_API
memcached_st* memcached_pool_destroy(memcached_pool_st* pool);
LIBMEMCACHED_API
memcached_st* memcached_pool_pop(memcached_pool_st* pool,
                                 bool block,
                                 memcached_return* rc);
LIBMEMCACHED_API
memcached_return memcached_pool_push(memcached_pool_st* pool, 
                                     memcached_st* mmc);

#ifdef __cplusplus
}
#endif

#endif /* MEMCACHED_POOL_H */
