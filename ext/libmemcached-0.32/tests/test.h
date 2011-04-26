/*
  Structures for generic tests.
*/
#ifdef	__cplusplus
extern "C" {
#endif

#include <libmemcached/memcached.h>
#include <stdio.h>

typedef struct world_st world_st;
typedef struct collection_st collection_st;
typedef struct test_st test_st;

typedef enum {
  TEST_SUCCESS= 0, /* Backwards compatibility */
  TEST_FAILURE,
  TEST_MEMORY_ALLOCATION_FAILURE,
  TEST_MAXIMUM_RETURN /* Always add new error code before */
} test_return;

struct test_st {
  const char *name;
  unsigned int requires_flush;
  test_return (*function)(memcached_st *memc);
};

struct collection_st {
  const char *name;
  memcached_return (*pre)(memcached_st *memc);
  memcached_return (*post)(memcached_st *memc);
  test_st *tests;
};

struct world_st {
  collection_st *collections;
  void *(*create)(void);
  void (*destroy)(void *collection_object);
};

/* How we make all of this work :) */
void get_world(world_st *world);

#ifdef	__cplusplus
}
#endif
