#include "libmemcached/common.h"
#include "libmemcached/memcached_pool.h"

#include <errno.h>
#include <pthread.h>

struct memcached_pool_st 
{
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  memcached_st *master;
  memcached_st **mmc;
  int firstfree;
  uint32_t size;
  uint32_t current_size;
};

static memcached_return mutex_enter(pthread_mutex_t *mutex) 
{
  int ret;
  do 
    ret= pthread_mutex_lock(mutex);
  while (ret == -1 && errno == EINTR);

  return (ret == -1) ? MEMCACHED_ERRNO : MEMCACHED_SUCCESS;
}

static memcached_return mutex_exit(pthread_mutex_t *mutex) {
  int ret;
  do
    ret= pthread_mutex_unlock(mutex);
  while (ret == -1 && errno == EINTR);

  return (ret == -1) ? MEMCACHED_ERRNO : MEMCACHED_SUCCESS;
}

/**
 * Grow the connection pool by creating a connection structure and clone the
 * original memcached handle.
 */
static int grow_pool(memcached_pool_st* pool) {
  memcached_st *obj= calloc(1, sizeof(*obj));
  if (obj == NULL)
    return -1;

  if (memcached_clone(obj, pool->master) == NULL)
  {
    free(obj);
    return -1;
  }

  pool->mmc[++pool->firstfree] = obj;
  pool->current_size++;

  return 0;
}

memcached_pool_st *memcached_pool_create(memcached_st* mmc, 
                                         uint32_t initial, uint32_t max) 
{
  memcached_pool_st* ret = NULL;
  memcached_pool_st object = { .mutex = PTHREAD_MUTEX_INITIALIZER, 
                               .cond = PTHREAD_COND_INITIALIZER,
                               .master = mmc,
                               .mmc = calloc(max, sizeof(memcached_st*)),
                               .firstfree = -1,
                               .size = max, 
                               .current_size = 0 };

  if (object.mmc != NULL) 
  {
    ret= calloc(1, sizeof(*ret));
    if (ret == NULL) 
    {
      free(object.mmc);
      return NULL;
    } 

    *ret = object;

    /* Try to create the initial size of the pool. An allocation failure at
     * this time is not fatal..
     */
    for (unsigned int ii=0; ii < initial; ++ii)
      if (grow_pool(ret) == -1)
        break;
  }

  return ret;
}

memcached_st*  memcached_pool_destroy(memcached_pool_st* pool) 
{
  memcached_st *ret = pool->master;

  for (int xx= 0; xx <= pool->firstfree; ++xx) 
  {
    memcached_free(pool->mmc[xx]);
    free(pool->mmc[xx]);
    pool->mmc[xx] = NULL;
  }
  
  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->cond);
  free(pool->mmc);
  free(pool);

  return ret;
}

memcached_st* memcached_pool_pop(memcached_pool_st* pool,
                                 bool block,
                                 memcached_return *rc) 
{
  memcached_st *ret= NULL;
  if ((*rc= mutex_enter(&pool->mutex)) != MEMCACHED_SUCCESS) 
    return NULL;

  do 
  {
    if (pool->firstfree > -1)
       ret= pool->mmc[pool->firstfree--];
    else if (pool->current_size == pool->size) 
    {
      if (!block) 
      {
        *rc= mutex_exit(&pool->mutex);
        return NULL;
      }

      if (pthread_cond_wait(&pool->cond, &pool->mutex) == -1) 
      {
        int err = errno;
        mutex_exit(&pool->mutex);
        errno = err; 
        *rc= MEMCACHED_ERRNO;
        return NULL;
      }
    } 
    else if (grow_pool(pool) == -1) 
    {
       *rc= mutex_exit(&pool->mutex);
       return NULL;
    }
  } 
  while (ret == NULL);

  *rc= mutex_exit(&pool->mutex);

  return ret;
}

memcached_return memcached_pool_push(memcached_pool_st* pool, 
                                     memcached_st *mmc)
{
  memcached_return rc= mutex_enter(&pool->mutex);

  if (rc != MEMCACHED_SUCCESS)
    return rc;

  pool->mmc[++pool->firstfree]= mmc;

  if (pool->firstfree == 0 && pool->current_size == pool->size) 
  {
    /* we might have people waiting for a connection.. wake them up :-) */
    pthread_cond_broadcast(&pool->cond);
  }

  return mutex_exit(&pool->mutex);
}
