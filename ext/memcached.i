%module memcached
%include "typemaps.i"
%{
#include <libmemcached/memcached.h>
%}

%include "memcached.h"

# extern memcached_st * memcached_create(memcached_st* OUTPUT);
# extern memcached_st * memcached_clone(memcached_st* OUTPUT, memcached_st* INPUT);

extern unsigned int memcached_server_count(memcached_st* INPUT);
extern memcached_return memcached_server_add(memcached_st* INPUT, char* hostname, unsigned int port);
# extern memcached_return memcached_server_add_unix_socket(memcached_st* INPUT, char* socket);
# extern void memcached_free(memcached_st* INPUT);