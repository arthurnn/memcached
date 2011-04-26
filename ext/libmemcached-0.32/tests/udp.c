/*
  Sample test application.
*/
#include <assert.h>
#include <libmemcached/memcached.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "test.h"
#include "server.h"

/* Prototypes */
test_return set_test(memcached_st *memc);
void *world_create(void);
void world_destroy(void *p);

test_return set_test(memcached_st *memc)
{
  memcached_return rc;
  const char *key= "foo";
  const char *value= "when we sanitize";

  rc= memcached_set(memc, key, strlen(key), 
                    value, strlen(value),
                    (time_t)0, (uint32_t)0);
  assert(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);

  return TEST_SUCCESS;
}

test_st tests[] ={
  {"set", 1, set_test },
  {0, 0, 0}
};

collection_st collection[] ={
  {"udp", 0, 0, tests},
  {0, 0, 0, 0}
};

#define SERVERS_TO_CREATE 1

void *world_create(void)
{
  server_startup_st *construct;

  construct= (server_startup_st *)malloc(sizeof(server_startup_st));
  memset(construct, 0, sizeof(server_startup_st));
  construct->count= SERVERS_TO_CREATE;
  construct->udp= 1;
  server_startup(construct);

  return construct;
}

void world_destroy(void *p)
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
