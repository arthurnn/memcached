/*
 * Summary: SASL support for memcached
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Trond Norbye
 */

#ifndef LIBMEMCACHED_MEMCACHED_SASL_H
#define LIBMEMCACHED_MEMCACHED_SASL_H

#ifdef LIBMEMCACHED_WITH_SASL_SUPPORT
#include <sasl/sasl.h>

#ifdef __cplusplus
extern "C" {
#endif

LIBMEMCACHED_API
void memcached_set_sasl_callbacks(memcached_st *ptr,
                                  const sasl_callback_t *callbacks);

LIBMEMCACHED_API
memcached_return  memcached_set_sasl_auth_data(memcached_st *ptr,
                                               const char *username,
                                               const char *password);

LIBMEMCACHED_API
memcached_return memcached_destroy_sasl_auth_data(memcached_st *ptr);


LIBMEMCACHED_API
const sasl_callback_t *memcached_get_sasl_callbacks(memcached_st *ptr);

LIBMEMCACHED_LOCAL
memcached_return memcached_sasl_authenticate_connection(memcached_server_st *server);

#ifdef __cplusplus
}
#endif

#endif /* LIBMEMCACHED_WITH_SASL_SUPPORT */

#endif /* LIBMEMCACHED_MEMCACHED_SASL_H */
