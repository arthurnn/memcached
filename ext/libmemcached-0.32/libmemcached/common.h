/*
  Common include file for libmemached
*/

#ifndef LIBMEMCACHED_COMMON_H
#define LIBMEMCACHED_COMMON_H

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* Note we need the 2 concats below because arguments to ##
 * are not expanded, so we need to expand __LINE__ with one indirection
 * before doing the actual concatenation.
 * From: http://www.pixelbeat.org/programming/gcc/static_assert.html
 */
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define assert_on_compile(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

/* Define this here, which will turn on the visibilty controls while we're
 * building libmemcached.
 */
#define BUILDING_LIBMEMCACHED 1


#include "libmemcached/memcached.h"
#include "libmemcached/memcached_watchpoint.h"

/* These are private not to be installed headers */
#include "libmemcached/memcached_io.h"
#include "libmemcached/memcached_internal.h"
#include "libmemcached/libmemcached_probes.h"
#include "libmemcached/memcached/protocol_binary.h"

/* string value */
struct memcached_continuum_item_st {
  uint32_t index;
  uint32_t value;
};


#if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)

#define likely(x)       if((x))
#define unlikely(x)     if((x))

#else

#define likely(x)       if(__builtin_expect(!!(x), 1))
#define unlikely(x)     if(__builtin_expect((x), 0))
#endif


#define MEMCACHED_BLOCK_SIZE 1024
#define MEMCACHED_DEFAULT_COMMAND_SIZE 350
#define SMALL_STRING_LEN 1024
#define HUGE_STRING_LEN 8196


typedef enum {
  MEM_NO_BLOCK= (1 << 0),
  MEM_TCP_NODELAY= (1 << 1),
  MEM_REUSE_MEMORY= (1 << 2),
  MEM_USE_MD5= (1 << 3),
  /* 4 was once Ketama */
  MEM_USE_CRC= (1 << 5),
  MEM_USE_CACHE_LOOKUPS= (1 << 6),
  MEM_SUPPORT_CAS= (1 << 7),
  MEM_BUFFER_REQUESTS= (1 << 8),
  MEM_USE_SORT_HOSTS= (1 << 9),
  MEM_VERIFY_KEY= (1 << 10),
  /* 11 used for weighted ketama */
  MEM_KETAMA_WEIGHTED= (1 << 11),
  MEM_BINARY_PROTOCOL= (1 << 12),
  MEM_HASH_WITH_PREFIX_KEY= (1 << 13),
  MEM_NOREPLY= (1 << 14),
  MEM_USE_UDP= (1 << 15),
  MEM_AUTO_EJECT_HOSTS= (1 << 16)
} memcached_flags;

/* Hashing algo */

LIBMEMCACHED_LOCAL
void md5_signature(const unsigned char *key, unsigned int length, unsigned char *result);
LIBMEMCACHED_LOCAL
uint32_t hash_crc32(const char *data,
                    size_t data_len);
#ifdef HAVE_HSIEH_HASH
LIBMEMCACHED_LOCAL
uint32_t hsieh_hash(const char *key, size_t key_length);
#endif
LIBMEMCACHED_LOCAL
uint32_t murmur_hash(const char *key, size_t key_length);
LIBMEMCACHED_LOCAL
uint32_t jenkins_hash(const void *key, size_t length, uint32_t initval);

LIBMEMCACHED_LOCAL
memcached_return memcached_connect(memcached_server_st *ptr);
LIBMEMCACHED_LOCAL
memcached_return memcached_response(memcached_server_st *ptr,
                                    char *buffer, size_t buffer_length,
                                    memcached_result_st *result);
LIBMEMCACHED_LOCAL
void memcached_quit_server(memcached_server_st *ptr, uint8_t io_death);

#define memcached_server_response_increment(A) (A)->cursor_active++
#define memcached_server_response_decrement(A) (A)->cursor_active--
#define memcached_server_response_reset(A) (A)->cursor_active=0

LIBMEMCACHED_LOCAL
memcached_return memcached_do(memcached_server_st *ptr, const void *commmand,
                              size_t command_length, uint8_t with_flush);

LIBMEMCACHED_LOCAL
memcached_return memcached_vdo(memcached_server_st *ptr,
                               const struct libmemcached_io_vector_st *vector, size_t count,
                               uint8_t with_flush);

LIBMEMCACHED_LOCAL
memcached_return value_fetch(memcached_server_st *ptr,
                             char *buffer,
                             memcached_result_st *result);
LIBMEMCACHED_LOCAL
void server_list_free(memcached_st *ptr, memcached_server_st *servers);

LIBMEMCACHED_LOCAL
memcached_return memcached_key_test(const char **keys, size_t *key_length,
                                    size_t number_of_keys);


LIBMEMCACHED_LOCAL
uint32_t generate_hash(memcached_st *ptr, const char *key, size_t key_length);

#ifndef HAVE_HTONLL
LIBMEMCACHED_LOCAL
extern uint64_t ntohll(uint64_t);
LIBMEMCACHED_LOCAL
extern uint64_t htonll(uint64_t);
#endif

LIBMEMCACHED_LOCAL
memcached_return memcached_purge(memcached_server_st *ptr);

static inline memcached_return memcached_validate_key_length(size_t key_length,
                                                             bool binary) {
  unlikely (key_length == 0)
    return MEMCACHED_BAD_KEY_PROVIDED;

  if (binary)
  {
    unlikely (key_length > 0xffff)
      return MEMCACHED_BAD_KEY_PROVIDED;
  }
  else
  {
    unlikely (key_length >= MEMCACHED_MAX_KEY)
      return MEMCACHED_BAD_KEY_PROVIDED;
  }

  return MEMCACHED_SUCCESS;
}

#endif /* LIBMEMCACHED_COMMON_H */
