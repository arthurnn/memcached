/*
  Memcached library
*/
#include "common.h"

memcached_st *memcached_create(memcached_st *ptr)
{
  memcached_result_st *result_ptr;

  if (ptr == NULL)
  {
    ptr= (memcached_st *)malloc(sizeof(memcached_st));

    if (!ptr)
      return NULL; /*  MEMCACHED_MEMORY_ALLOCATION_FAILURE */

    memset(ptr, 0, sizeof(memcached_st));
    ptr->is_allocated= MEMCACHED_ALLOCATED;
  }
  else
  {
    memset(ptr, 0, sizeof(memcached_st));
  }
  result_ptr= memcached_result_create(ptr, &ptr->result);
  WATCHPOINT_ASSERT(result_ptr);
  ptr->poll_timeout= MEMCACHED_DEFAULT_TIMEOUT;
  ptr->connect_timeout= MEMCACHED_DEFAULT_TIMEOUT;
  ptr->retry_timeout= 0;
  ptr->distribution= MEMCACHED_DISTRIBUTION_MODULA;

  /* TODO, Document why we picked these defaults */
  ptr->io_msg_watermark= 500;
  ptr->io_bytes_watermark= 65 * 1024;

  return ptr;
}

void memcached_free(memcached_st *ptr)
{
  /* If we have anything open, lets close it now */
  memcached_quit(ptr);
  server_list_free(ptr, ptr->hosts);
  memcached_result_free(&ptr->result);

  if (ptr->on_cleanup)
    ptr->on_cleanup(ptr);

  if (ptr->continuum)
  {
    if (ptr->call_free)
      ptr->call_free(ptr, ptr->continuum);
    else
      free(ptr->continuum);
  }

  if (ptr->is_allocated == MEMCACHED_ALLOCATED)
  {
    if (ptr->call_free)
      ptr->call_free(ptr, ptr);
    else
      free(ptr);
  }
  else
    ptr->is_allocated= MEMCACHED_USED;
}

/*
  clone is the destination, while source is the structure to clone.
  If source is NULL the call is the same as if a memcached_create() was
  called.
*/
memcached_st *memcached_clone(memcached_st *clone, memcached_st *source)
{
  memcached_return rc= MEMCACHED_SUCCESS;
  memcached_st *new_clone;

  if (source == NULL)
    return memcached_create(clone);

  if (clone && clone->is_allocated == MEMCACHED_USED)
  {
    return NULL;
  }
  
  new_clone= memcached_create(clone);
  
  if (new_clone == NULL)
    return NULL;

  if (source->hosts)
    rc= memcached_server_push(new_clone, source->hosts);

  if (rc != MEMCACHED_SUCCESS)
  {
    memcached_free(new_clone);

    return NULL;
  }


  new_clone->flags= source->flags;
  new_clone->send_size= source->send_size;
  new_clone->recv_size= source->recv_size;
  new_clone->poll_timeout= source->poll_timeout;
  new_clone->connect_timeout= source->connect_timeout;
  new_clone->retry_timeout= source->retry_timeout;
  new_clone->distribution= source->distribution;
  new_clone->hash= source->hash;
  new_clone->hash_continuum= source->hash_continuum;
  new_clone->user_data= source->user_data;

  new_clone->snd_timeout= source->snd_timeout;
  new_clone->rcv_timeout= source->rcv_timeout;

  new_clone->on_clone= source->on_clone;
  new_clone->on_cleanup= source->on_cleanup;
  new_clone->call_free= source->call_free;
  new_clone->call_malloc= source->call_malloc;
  new_clone->call_realloc= source->call_realloc;
  new_clone->get_key_failure= source->get_key_failure;
  new_clone->delete_trigger= source->delete_trigger;

  if (source->prefix_key[0] != 0)
  {
    strcpy(new_clone->prefix_key, source->prefix_key);
    new_clone->prefix_key_length= source->prefix_key_length;
  }

  rc= run_distribution(new_clone);
  if (rc != MEMCACHED_SUCCESS)
  {
    memcached_free(new_clone);

    return NULL;
  }

  if (source->on_clone)
    source->on_clone(source, new_clone);

  return new_clone;
}
