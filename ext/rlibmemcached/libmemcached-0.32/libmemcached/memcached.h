/*
 * Summary: interface for memcached server
 * Description: main include file for libmemcached
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef LIBMEMCACHED_MEMCACHED_H
#define LIBMEMCACHED_MEMCACHED_H

#include <stdlib.h>
#include <inttypes.h>
#if !defined(__cplusplus)
# include <stdbool.h>
#endif
#include <sys/types.h>
#include <netinet/in.h>

#include <libmemcached/visibility.h>
#include <libmemcached/memcached_configure.h>
#include <libmemcached/memcached_constants.h>
#include <libmemcached/memcached_types.h>
#include <libmemcached/memcached_get.h>
#include <libmemcached/memcached_touch.h>
#include <libmemcached/memcached_server.h>
#include <libmemcached/memcached_string.h>
#include <libmemcached/memcached_result.h>
#include <libmemcached/memcached_storage.h>
#include <libmemcached/memcached_exist.h>
#include <libmemcached/memcached_sasl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEMCACHED_VERSION_STRING_LENGTH 24
#define LIBMEMCACHED_VERSION_STRING "0.32"

struct memcached_analysis_st {
  uint32_t average_item_size;
  uint32_t longest_uptime;
  uint32_t least_free_server;
  uint32_t most_consumed_server;
  uint32_t oldest_server;
  double pool_hit_ratio;
  uint64_t most_used_bytes;
  uint64_t least_remaining_bytes;
};

struct memcached_stat_st {
  uint32_t connection_structures;
  uint32_t curr_connections;
  uint32_t curr_items;
  uint32_t pid;
  uint32_t pointer_size;
  uint32_t rusage_system_microseconds;
  uint32_t rusage_system_seconds;
  uint32_t rusage_user_microseconds;
  uint32_t rusage_user_seconds;
  uint32_t threads;
  uint32_t time;
  uint32_t total_connections;
  uint32_t total_items;
  uint32_t uptime;
  uint64_t bytes;
  uint64_t bytes_read;
  uint64_t bytes_written;
  uint64_t cmd_get;
  uint64_t cmd_set;
  uint64_t evictions;
  uint64_t get_hits;
  uint64_t get_misses;
  uint64_t limit_maxbytes;
  char version[MEMCACHED_VERSION_STRING_LENGTH];
};

struct memcached_st {
  uint8_t purging;
  bool is_allocated;
  uint8_t distribution;
  uint8_t hash;
  uint32_t continuum_points_counter;
  memcached_server_st *hosts;
  int32_t snd_timeout;
  int32_t rcv_timeout;
  int32_t poll_max_retries;
  uint32_t server_failure_limit;
  uint32_t io_msg_watermark;
  uint32_t io_bytes_watermark;
  uint32_t io_key_prefetch;
  uint32_t number_of_hosts;
  uint32_t cursor_server;
  int cached_errno;
  uint32_t flags;
  int32_t poll_timeout;
  int32_t connect_timeout;
  int32_t retry_timeout;
  uint32_t continuum_count;
  int send_size;
  int recv_size;
  void *user_data;
  time_t next_distribution_rebuild;
  size_t prefix_key_length;
  memcached_hash hash_continuum;
  memcached_result_st result;
  memcached_continuum_item_st *continuum;
  memcached_clone_func on_clone;
  memcached_cleanup_func on_cleanup;
  memcached_free_function call_free;
  memcached_malloc_function call_malloc;
  memcached_realloc_function call_realloc;
  memcached_calloc_function call_calloc;
  memcached_trigger_key get_key_failure;
  memcached_trigger_delete_key delete_trigger;
  char prefix_key[MEMCACHED_PREFIX_KEY_MAX_SIZE];
  uint32_t number_of_live_hosts;
  uint32_t *live_host_indices;
  uint32_t live_host_indices_size;
#ifdef LIBMEMCACHED_WITH_SASL_SUPPORT
  const sasl_callback_t *sasl_callbacks;
#endif
  int last_server_key;
};

LIBMEMCACHED_API
memcached_return memcached_version(memcached_st *ptr);

/* Public API */

LIBMEMCACHED_API
const char * memcached_lib_version(void);

LIBMEMCACHED_API
memcached_st *memcached_create(memcached_st *ptr);
LIBMEMCACHED_API
void memcached_free(memcached_st *ptr);
LIBMEMCACHED_API
memcached_st *memcached_clone(memcached_st *clone, memcached_st *ptr);

LIBMEMCACHED_API
memcached_return memcached_delete(memcached_st *ptr, const char *key, size_t key_length,
                                  time_t expiration);
LIBMEMCACHED_API
memcached_return memcached_increment(memcached_st *ptr,
                                     const char *key, size_t key_length,
                                     uint32_t offset,
                                     uint64_t *value);
LIBMEMCACHED_API
memcached_return memcached_decrement(memcached_st *ptr,
                                     const char *key, size_t key_length,
                                     uint32_t offset,
                                     uint64_t *value);
LIBMEMCACHED_API
memcached_return memcached_increment_with_initial(memcached_st *ptr,
                                                  const char *key,
                                                  size_t key_length,
                                                  uint64_t offset,
                                                  uint64_t initial,
                                                  time_t expiration,
                                                  uint64_t *value);
LIBMEMCACHED_API
memcached_return memcached_decrement_with_initial(memcached_st *ptr,
                                                  const char *key,
                                                  size_t key_length,
                                                  uint64_t offset,
                                                  uint64_t initial,
                                                  time_t expiration,
                                                  uint64_t *value);
LIBMEMCACHED_API
void memcached_stat_free(memcached_st *, memcached_stat_st *);
LIBMEMCACHED_API
memcached_stat_st *memcached_stat(memcached_st *ptr, char *args, memcached_return *error);
LIBMEMCACHED_API
memcached_return memcached_stat_servername(memcached_stat_st *memc_stat, char *args,
                                           char *hostname, unsigned int port);
LIBMEMCACHED_API
memcached_return memcached_flush(memcached_st *ptr, time_t expiration);
LIBMEMCACHED_API
memcached_return memcached_verbosity(memcached_st *ptr, unsigned int verbosity);
LIBMEMCACHED_API
void memcached_quit(memcached_st *ptr);
LIBMEMCACHED_API
const char *memcached_strerror(memcached_st *ptr, memcached_return rc);
LIBMEMCACHED_API
memcached_return memcached_behavior_set(memcached_st *ptr, memcached_behavior flag, uint64_t data);
LIBMEMCACHED_API
uint64_t memcached_behavior_get(memcached_st *ptr, memcached_behavior flag);

/* The two public hash bits */
LIBMEMCACHED_API
uint32_t memcached_generate_hash_value(const char *key, size_t key_length, memcached_hash hash_algorithm);
LIBMEMCACHED_API
uint32_t memcached_generate_hash(memcached_st *ptr, const char *key, size_t key_length);

LIBMEMCACHED_API
memcached_return memcached_flush_buffers(memcached_st *mem);

/* Server Public functions */

LIBMEMCACHED_API
memcached_return memcached_server_add_udp(memcached_st *ptr,
                                          const char *hostname,
                                          unsigned int port);
LIBMEMCACHED_API
memcached_return memcached_server_add_unix_socket(memcached_st *ptr,
                                                  const char *filename);
LIBMEMCACHED_API
memcached_return memcached_server_add(memcached_st *ptr, const char *hostname,
                                      unsigned int port);

LIBMEMCACHED_API
memcached_return memcached_server_add_udp_with_weight(memcached_st *ptr,
                                                      const char *hostname,
                                                      unsigned int port,
                                                      uint32_t weight);
LIBMEMCACHED_API
memcached_return memcached_server_add_unix_socket_with_weight(memcached_st *ptr,
                                                              const char *filename,
                                                              uint32_t weight);
LIBMEMCACHED_API
memcached_return memcached_server_add_with_weight(memcached_st *ptr, const char *hostname,
                                                  unsigned int port,
                                                  uint32_t weight);
LIBMEMCACHED_API
void memcached_server_list_free(memcached_server_st *ptr);
LIBMEMCACHED_API
memcached_return memcached_server_push(memcached_st *ptr, memcached_server_st *list);

LIBMEMCACHED_API
memcached_server_st *memcached_server_list_append(memcached_server_st *ptr,
                                                  const char *hostname,
                                                  unsigned int port,
                                                  memcached_return *error);
LIBMEMCACHED_API
memcached_server_st *memcached_server_list_append_with_weight(memcached_server_st *ptr,
                                                              const char *hostname,
                                                              unsigned int port,
                                                              uint32_t weight,
                                                              memcached_return *error);
LIBMEMCACHED_API
unsigned int memcached_server_list_count(memcached_server_st *ptr);
LIBMEMCACHED_API
memcached_server_st *memcached_servers_parse(const char *server_strings);

LIBMEMCACHED_API
char *memcached_stat_get_value(memcached_st *ptr, memcached_stat_st *memc_stat,
                               const char *key, memcached_return *error);
LIBMEMCACHED_API
char ** memcached_stat_get_keys(memcached_st *ptr, memcached_stat_st *memc_stat,
                                memcached_return *error);

LIBMEMCACHED_API
memcached_return memcached_delete_by_key(memcached_st *ptr,
                                         const char *master_key, size_t master_key_length,
                                         const char *key, size_t key_length,
                                         time_t expiration);

LIBMEMCACHED_API
memcached_return memcached_fetch_execute(memcached_st *ptr,
                                             memcached_execute_function *callback,
                                             void *context,
                                             unsigned int number_of_callbacks);

LIBMEMCACHED_API
memcached_return memcached_callback_set(memcached_st *ptr,
                                        memcached_callback flag,
                                        void *data);
LIBMEMCACHED_API
void *memcached_callback_get(memcached_st *ptr,
                             memcached_callback flag,
                             memcached_return *error);

LIBMEMCACHED_API
memcached_return memcached_dump(memcached_st *ptr, memcached_dump_func *function, void *context, uint32_t number_of_callbacks);


LIBMEMCACHED_API
memcached_return memcached_set_memory_allocators(memcached_st *ptr,
                                                 memcached_malloc_function mem_malloc,
                                                 memcached_free_function mem_free,
                                                 memcached_realloc_function mem_realloc,
                                                 memcached_calloc_function mem_calloc);

LIBMEMCACHED_API
void memcached_get_memory_allocators(memcached_st *ptr,
                                     memcached_malloc_function *mem_malloc,
                                     memcached_free_function *mem_free,
                                     memcached_realloc_function *mem_realloc,
                                     memcached_calloc_function *mem_calloc);

LIBMEMCACHED_API
void *memcached_get_user_data(memcached_st *ptr);
LIBMEMCACHED_API
void *memcached_set_user_data(memcached_st *ptr, void *data);

LIBMEMCACHED_API
memcached_return run_distribution(memcached_st *ptr);
#ifdef __cplusplus
}
#endif


#endif /* LIBMEMCACHED_MEMCACHED_H */
