/*
  This is a partial implementation for fetching/creating memcached_server_st objects.
*/
#include "common.h"

memcached_server_st *memcached_server_create(memcached_st *memc, memcached_server_st *ptr)
{
  if (ptr == NULL)
  {
    ptr= (memcached_server_st *)malloc(sizeof(memcached_server_st));

    if (!ptr)
      return NULL; /*  MEMCACHED_MEMORY_ALLOCATION_FAILURE */

    memset(ptr, 0, sizeof(memcached_server_st));
    ptr->is_allocated= MEMCACHED_ALLOCATED;
  }
  else
  {
    memset(ptr, 0, sizeof(memcached_server_st));
  }
  
  ptr->root= memc;

  return ptr;
}

void memcached_server_free(memcached_server_st *ptr)
{
  memcached_return rc;
  WATCHPOINT_ASSERT(ptr->is_allocated != MEMCACHED_NOT_ALLOCATED);

  rc= memcached_io_close(ptr);
  WATCHPOINT_ASSERT(rc == MEMCACHED_SUCCESS);

  if (ptr->address_info)
  {
    freeaddrinfo(ptr->address_info);
    ptr->address_info= NULL;
  }

  if (ptr->is_allocated == MEMCACHED_ALLOCATED)
  {
    if (ptr->root && ptr->root->call_free)
      ptr->root->call_free(ptr->root, ptr);
    else
      free(ptr);
  }
  else
    ptr->is_allocated= MEMCACHED_USED;
}

/*
  If we do not have a valid object to clone from, we toss an error.
*/
memcached_server_st *memcached_server_clone(memcached_server_st *clone, memcached_server_st *ptr)
{
  memcached_server_st *new_clone;

  /* We just do a normal create if ptr is missing */
  if (ptr == NULL)
    return NULL;

  if (clone && clone->is_allocated == MEMCACHED_USED)
  {
    WATCHPOINT_ASSERT(0);
    return NULL;
  }
  
  new_clone= memcached_server_create(ptr->root, clone);
  
  if (new_clone == NULL)
    return NULL;

  new_clone->root= ptr->root;

  host_reset(new_clone->root, new_clone, 
             ptr->hostname, ptr->port, ptr->weight,
             ptr->type);

  return new_clone;
}

memcached_return memcached_server_cursor(memcached_st *ptr, 
                                         memcached_server_function *callback,
                                         void *context,
                                         unsigned int number_of_callbacks)
{
  unsigned int y;

  for (y= 0; y < ptr->number_of_hosts; y++)
  {
    unsigned int x;

    for (x= 0; x < number_of_callbacks; x++)
    {
      unsigned int iferror;

      iferror= (*callback[x])(ptr, &ptr->hosts[y], context);

      if (iferror)
        continue;
    }
  }

  return MEMCACHED_SUCCESS;
}

memcached_server_st *memcached_server_by_key(memcached_st *ptr,  const char *key, size_t key_length, memcached_return *error)
{
  uint32_t server_key;

  unlikely (key_length == 0)
  {
    *error= MEMCACHED_NO_KEY_PROVIDED;
    return NULL;
  }

  unlikely (ptr->number_of_hosts == 0)
  {
    *error= MEMCACHED_NO_SERVERS;
    return NULL;
  }

  if ((ptr->flags & MEM_VERIFY_KEY) && (memcachd_key_test((char **)&key, &key_length, 1) == MEMCACHED_BAD_KEY_PROVIDED))
  {
    *error= MEMCACHED_BAD_KEY_PROVIDED;
    return NULL;
  }

  server_key= memcached_generate_hash(ptr, key, key_length);

  return memcached_server_clone(NULL, &ptr->hosts[server_key]);

}
