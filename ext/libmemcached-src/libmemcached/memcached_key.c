#include "common.h"

memcached_return memcachd_key_test(char **keys, size_t *key_length, 
                                   unsigned int number_of_keys)
{
  uint32_t x;

  for (x= 0; x < number_of_keys; x++)
  {
    size_t y;

    if (*(key_length + x) == 0)
        return MEMCACHED_BAD_KEY_PROVIDED;

    for (y= 0; y < *(key_length + x); y++)
    {
      if ((isgraph(keys[x][y])) == 0)
        return MEMCACHED_BAD_KEY_PROVIDED;
    }
  }

  return MEMCACHED_SUCCESS;
}

