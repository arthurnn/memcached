/* 
  memcached_result_st are used to internally represent the return values from
  memcached. We use a structure so that long term as identifiers are added 
  to memcached we will be able to absorb new attributes without having 
  to addjust the entire API.
*/
#include "common.h"

memcached_result_st *memcached_result_create(memcached_st *memc, 
                                             memcached_result_st *ptr)
{
  /* Saving malloc calls :) */
  if (ptr)
    memset(ptr, 0, sizeof(memcached_result_st));
  else
  {
    ptr= memc->call_malloc(memc, sizeof(memcached_result_st));

    if (ptr == NULL)
      return NULL;
    ptr->is_allocated= true;
  }

  ptr->root= memc;
  memcached_string_create(memc, &ptr->value, 0);
  WATCHPOINT_ASSERT(ptr->value.string == NULL);

  return ptr;
}

void memcached_result_reset(memcached_result_st *ptr)
{
  ptr->key_length= 0;
  memcached_string_reset(&ptr->value);
  ptr->flags= 0;
  ptr->cas= 0;
  ptr->expiration= 0;
}

/*
  NOTE turn into macro
*/
memcached_return memcached_result_set_value(memcached_result_st *ptr, const char *value, size_t length)
{
  return memcached_string_append(&ptr->value, value, length);
}

void memcached_result_free(memcached_result_st *ptr)
{
  if (ptr == NULL)
    return;

  memcached_string_free(&ptr->value);

  if (ptr->is_allocated)
    free(ptr);
}
