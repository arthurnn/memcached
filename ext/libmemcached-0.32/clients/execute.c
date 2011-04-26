/*
  Execute a memcached_set() a set of pairs.
  Return the number of rows set.
*/

#include "libmemcached/common.h"

#include "execute.h"

unsigned int execute_set(memcached_st *memc, pairs_st *pairs, unsigned int number_of)
{
  memcached_return rc;
  unsigned int x;
  unsigned int pairs_sent;

  for (x= 0, pairs_sent= 0; x < number_of; x++)
  {
    rc= memcached_set(memc, pairs[x].key, pairs[x].key_length,
                      pairs[x].value, pairs[x].value_length,
                      0, 0);
    if (rc != MEMCACHED_SUCCESS && rc != MEMCACHED_BUFFERED)
      fprintf(stderr, "Failured on insert of %.*s\n", 
              (unsigned int)pairs[x].key_length, pairs[x].key);
    else
      pairs_sent++;
  }

  return pairs_sent;
}

/*
  Execute a memcached_get() on a set of pairs.
  Return the number of rows retrieved.
*/
unsigned int execute_get(memcached_st *memc, pairs_st *pairs, unsigned int number_of)
{
  memcached_return rc;
  unsigned int x;
  unsigned int retrieved;


  for (retrieved= 0,x= 0; x < number_of; x++)
  {
    char *value;
    size_t value_length;
    uint32_t flags;
    unsigned int fetch_key;

    fetch_key= (unsigned int)random() % number_of;

    value= memcached_get(memc, pairs[fetch_key].key, pairs[fetch_key].key_length,
                         &value_length, &flags, &rc);

    if (rc != MEMCACHED_SUCCESS)
      fprintf(stderr, "Failured on read of %.*s\n", 
              (unsigned int)pairs[fetch_key].key_length, pairs[fetch_key].key);
    else
      retrieved++;

    free(value);
  }

  return retrieved;
}
