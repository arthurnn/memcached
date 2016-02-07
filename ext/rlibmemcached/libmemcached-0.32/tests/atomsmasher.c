/*
  Sample test application.
*/
#include "libmemcached/common.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "server.h"
#include "../clients/generator.h"
#include "../clients/execute.h"

#ifndef INT64_MAX
#define INT64_MAX LONG_MAX
#endif
#ifndef INT32_MAX
#define INT32_MAX INT_MAX
#endif


#include "test.h"

/* Number of items generated for tests */
#define GLOBAL_COUNT 100000

/* Number of times to run the test loop */
#define TEST_COUNTER 500000
static uint32_t global_count;

static pairs_st *global_pairs;
static char *global_keys[GLOBAL_COUNT];
static size_t global_keys_length[GLOBAL_COUNT];

static test_return cleanup_pairs(memcached_st *memc __attribute__((unused)))
{
  pairs_free(global_pairs);

  return 0;
}

static test_return generate_pairs(memcached_st *memc __attribute__((unused)))
{
  unsigned long long x;
  global_pairs= pairs_generate(GLOBAL_COUNT, 400);
  global_count= GLOBAL_COUNT;

  for (x= 0; x < global_count; x++)
  {
    global_keys[x]= global_pairs[x].key; 
    global_keys_length[x]=  global_pairs[x].key_length;
  }

  return 0;
}

static test_return drizzle(memcached_st *memc)
{
  unsigned int x;
  memcached_return rc;
  char *return_value;
  size_t return_value_length;
  uint32_t flags;

infinite:
  for (x= 0; x < TEST_COUNTER; x++)
  {
    uint32_t test_bit;
    uint8_t which;

    test_bit= (uint32_t)(random() % GLOBAL_COUNT);
    which= (uint8_t)(random() % 2);

    if (which == 0)
    {
      return_value= memcached_get(memc, global_keys[test_bit], global_keys_length[test_bit],
                                  &return_value_length, &flags, &rc);
      if (rc == MEMCACHED_SUCCESS && return_value)
        free(return_value);
      else if (rc == MEMCACHED_NOTFOUND)
        continue;
      else
      {
        WATCHPOINT_ERROR(rc);
        WATCHPOINT_ASSERT(rc);
      }
    } 
    else
    {
      rc= memcached_set(memc, global_pairs[test_bit].key, 
                        global_pairs[test_bit].key_length,
                        global_pairs[test_bit].value, 
                        global_pairs[test_bit].value_length,
                        0, 0);
      if (rc != MEMCACHED_SUCCESS && rc != MEMCACHED_BUFFERED)
      {
        WATCHPOINT_ERROR(rc);
        WATCHPOINT_ASSERT(0);
      }
    }
  }

  if (getenv("MEMCACHED_ATOM_BURIN_IN"))
    goto infinite;

  return 0;
}

static memcached_return pre_nonblock(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 0);

  return MEMCACHED_SUCCESS;
}

static memcached_return pre_md5(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_MD5);

  return MEMCACHED_SUCCESS;
}

static memcached_return pre_hsieh(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_HSIEH);

  return MEMCACHED_SUCCESS;
}

static memcached_return enable_consistent(memcached_st *memc)
{
  memcached_server_distribution value= MEMCACHED_DISTRIBUTION_CONSISTENT;
  memcached_hash hash;
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, value);
  pre_hsieh(memc);

  value= (memcached_server_distribution)memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION);
  assert(value == MEMCACHED_DISTRIBUTION_CONSISTENT);

  hash= (memcached_hash)memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_HASH);
  assert(hash == MEMCACHED_HASH_HSIEH);


  return MEMCACHED_SUCCESS;
}

/* 
  Set the value, then quit to make sure it is flushed.
  Come back in and test that add fails.
*/
static test_return add_test(memcached_st *memc)
{
  memcached_return rc;
  const char *key= "foo";
  const char *value= "when we sanitize";
  unsigned long long setting_value;

  setting_value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NO_BLOCK);

  rc= memcached_set(memc, key, strlen(key), 
                    value, strlen(value),
                    (time_t)0, (uint32_t)0);
  assert(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  memcached_quit(memc);
  rc= memcached_add(memc, key, strlen(key), 
                    value, strlen(value),
                    (time_t)0, (uint32_t)0);

  /* Too many broken OS'es have broken loopback in async, so we can't be sure of the result */
  if (setting_value)
    assert(rc == MEMCACHED_NOTSTORED || rc == MEMCACHED_STORED);
  else
    assert(rc == MEMCACHED_NOTSTORED);

  return 0;
}

/*
 * repeating add_tests many times
 * may show a problem in timing
 */
static test_return many_adds(memcached_st *memc)
{
  unsigned int i;
  for (i = 0; i < TEST_COUNTER; i++)
  {
    add_test(memc);
  }
  return 0;
}

test_st smash_tests[] ={
  {"generate_pairs", 1, generate_pairs },
  {"drizzle", 1, drizzle },
  {"cleanup", 1, cleanup_pairs },
  {"many_adds", 1, many_adds },
  {0, 0, 0}
};


collection_st collection[] ={
  {"smash", 0, 0, smash_tests},
  {"smash_hsieh", pre_hsieh, 0, smash_tests},
  {"smash_hsieh_consistent", enable_consistent, 0, smash_tests},
  {"smash_md5", pre_md5, 0, smash_tests},
  {"smash_nonblock", pre_nonblock, 0, smash_tests},
  {0, 0, 0, 0}
};

#define SERVERS_TO_CREATE 5

static void *world_create(void)
{
  server_startup_st *construct;

  construct= (server_startup_st *)malloc(sizeof(server_startup_st));
  memset(construct, 0, sizeof(server_startup_st));
  construct->count= SERVERS_TO_CREATE;
  construct->udp= 0;
  server_startup(construct);

  return construct;
}

static void world_destroy(void *p)
{
  server_startup_st *construct= (server_startup_st *)p;
  memcached_server_st *servers= (memcached_server_st *)construct->servers;
  memcached_server_list_free(servers);

  server_shutdown(construct);
  free(construct);
}

void get_world(world_st *world)
{
  world->collections= collection;
  world->create= world_create;
  world->destroy= world_destroy;
}
