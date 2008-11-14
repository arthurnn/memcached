/* Server IO, Not public! */
#include <memcached.h>

#ifndef __MEMCACHED_IO_H__
#define __MEMCACHED_IO_H__

ssize_t memcached_io_write(memcached_server_st *ptr,
                           const void *buffer, size_t length, char with_flush);
void memcached_io_reset(memcached_server_st *ptr);
ssize_t memcached_io_read(memcached_server_st *ptr,
                          void *buffer, size_t length);
memcached_return memcached_io_close(memcached_server_st *ptr);
void memcached_purge(memcached_server_st *ptr);
#endif /* __MEMCACHED_IO_H__ */
