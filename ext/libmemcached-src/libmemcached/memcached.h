/*
 * Summary: interface for memcached server
 * Description: main include file for libmemcached
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Brian Aker
 */

#ifndef __MEMCACHED_H__
#define __MEMCACHED_H__

#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <netinet/in.h>

#ifdef MEMCACHED_INTERNAL
#include <libmemcached/libmemcached_config.h>
#endif
#include <libmemcached/memcached_constants.h>
#include <libmemcached/memcached_types.h>
#include <libmemcached/memcached_watchpoint.h>
#include <libmemcached/memcached_get.h>
#include <libmemcached/memcached_server.h>
#include <libmemcached/memcached_string.h>
#include <libmemcached/memcached_result.h>

#ifdef __cplusplus
extern "C" {
#endif

/* These are Private and should not be used by applications */
#define MEMCACHED_VERSION_STRING_LENGTH 12

/* string value */
struct memcached_continuum_item_st {
  uint32_t index;
  uint32_t value;
};

#define LIBMEMCACHED_VERSION_STRING "0.25"

struct memcached_stat_st {
  uint32_t pid;
  uint32_t uptime;
  uint32_t threads;
  uint32_t time;
  uint32_t pointer_size;
  uint32_t rusage_user_seconds;
  uint32_t rusage_user_microseconds;
  uint32_t rusage_system_seconds;
  uint32_t rusage_system_microseconds;
  uint32_t curr_items;
  uint32_t total_items;
  uint64_t limit_maxbytes;
  uint32_t curr_connections;
  uint32_t total_connections;
  uint32_t connection_structures;
  uint64_t bytes;
  uint64_t cmd_get;
  uint64_t cmd_set;
  uint64_t get_hits;
  uint64_t get_misses;
  uint64_t evictions;
  uint64_t bytes_read;
  uint64_t bytes_written;
  char version[MEMCACHED_VERSION_STRING_LENGTH];
};

struct memcached_st {
  memcached_allocated is_allocated;
  memcached_server_st *hosts;
  uint32_t number_of_hosts;
  uint32_t cursor_server;
  int cached_errno;
  uint32_t flags;
  int send_size;
  int recv_size;
  int32_t poll_timeout;
  int32_t connect_timeout;
  int32_t retry_timeout;
  memcached_result_st result;
  memcached_hash hash;
  memcached_server_distribution distribution;
  void *user_data;
  uint32_t continuum_count;
  memcached_continuum_item_st *continuum;
  memcached_clone_func on_clone;
  memcached_cleanup_func on_cleanup;
  memcached_free_function call_free;
  memcached_malloc_function call_malloc;
  memcached_realloc_function call_realloc;
  memcached_trigger_key get_key_failure;
  memcached_trigger_delete_key delete_trigger;
  char prefix_key[MEMCACHED_PREFIX_KEY_MAX_SIZE];
  size_t prefix_key_length;
  memcached_hash hash_continuum;
  uint32_t continuum_points_counter;
  int32_t snd_timeout;
  int32_t rcv_timeout;
  uint32_t server_failure_limit;
  uint32_t io_msg_watermark;
  uint32_t io_bytes_watermark;
  char purging;
};


/* Public API */
const char * memcached_lib_version(void);

memcached_st *memcached_create(memcached_st *ptr);
void memcached_free(memcached_st *ptr);
memcached_st *memcached_clone(memcached_st *clone, memcached_st *ptr);

memcached_return memcached_delete(memcached_st *ptr, const char *key, size_t key_length,
                                  time_t expiration);
memcached_return memcached_increment(memcached_st *ptr, 
                                     const char *key, size_t key_length,
                                     uint32_t offset,
                                     uint64_t *value);
memcached_return memcached_decrement(memcached_st *ptr, 
                                     const char *key, size_t key_length,
                                     uint32_t offset,
                                     uint64_t *value);
void memcached_stat_free(memcached_st *, memcached_stat_st *);
memcached_stat_st *memcached_stat(memcached_st *ptr, char *args, memcached_return *error);
memcached_return memcached_stat_servername(memcached_stat_st *stat, char *args, 
                                           char *hostname, unsigned int port);
memcached_return memcached_flush(memcached_st *ptr, time_t expiration);
memcached_return memcached_verbosity(memcached_st *ptr, unsigned int verbosity);
void memcached_quit(memcached_st *ptr);
char *memcached_strerror(memcached_st *ptr, memcached_return rc);
memcached_return memcached_behavior_set(memcached_st *ptr, memcached_behavior flag, uint64_t data);
uint64_t memcached_behavior_get(memcached_st *ptr, memcached_behavior flag);

/* Server Public functions */

memcached_return memcached_server_add_udp(memcached_st *ptr, 
                                          const char *hostname,
                                          unsigned int port);
memcached_return memcached_server_add_unix_socket(memcached_st *ptr, 
                                                  const char *filename);
memcached_return memcached_server_add(memcached_st *ptr, const char *hostname, 
                                      unsigned int port);

memcached_return memcached_server_add_udp_with_weight(memcached_st *ptr, 
                                                      const char *hostname,
                                                      unsigned int port,
                                                      uint32_t weight);
memcached_return memcached_server_add_unix_socket_with_weight(memcached_st *ptr, 
                                                              const char *filename,
                                                              uint32_t weight);
memcached_return memcached_server_add_with_weight(memcached_st *ptr, const char *hostname, 
                                                  unsigned int port,
                                                  uint32_t weight);
void memcached_server_list_free(memcached_server_st *ptr);
memcached_return memcached_server_push(memcached_st *ptr, memcached_server_st *list);

memcached_server_st *memcached_server_list_append(memcached_server_st *ptr, 
                                                  const char *hostname, 
                                                  unsigned int port, 
                                                  memcached_return *error);
memcached_server_st *memcached_server_list_append_with_weight(memcached_server_st *ptr, 
                                                              const char *hostname, 
                                                              unsigned int port, 
                                                              uint32_t weight,
                                                              memcached_return *error);
unsigned int memcached_server_list_count(memcached_server_st *ptr);
memcached_server_st *memcached_servers_parse(char *server_strings);

char *memcached_stat_get_value(memcached_st *ptr, memcached_stat_st *stat, 
                               char *key, memcached_return *error);
char ** memcached_stat_get_keys(memcached_st *ptr, memcached_stat_st *stat, 
                                memcached_return *error);

memcached_return memcached_delete_by_key(memcached_st *ptr, 
                                         const char *master_key, size_t master_key_length,
                                         const char *key, size_t key_length,
                                         time_t expiration);

memcached_return memcached_fetch_execute(memcached_st *ptr, 
                                             memcached_execute_function *callback,
                                             void *context,
                                             unsigned int number_of_callbacks);

memcached_return memcached_callback_set(memcached_st *ptr, 
                                        memcached_callback flag, 
                                        void *data);
void *memcached_callback_get(memcached_st *ptr, 
                             memcached_callback flag,
                             memcached_return *error);


#ifdef __cplusplus
}
#endif

#include <libmemcached/memcached_storage.h>

#endif /* __MEMCACHED_H__ */
