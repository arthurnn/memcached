/* 
  "Murmur" hash provided by Austin, tanjent@gmail.com
  http://murmurhash.googlepages.com/

  Note - This code makes a few assumptions about how your machine behaves -

  1. We can read a 4-byte value from any address without crashing
  2. sizeof(int) == 4

  And it has a few limitations -
  1. It will not work incrementally.
  2. It will not produce the same results on little-endian and big-endian
  machines.

  Updated to murmur2 hash - BP
*/

#include "common.h"

uint32_t murmur_hash(const char *key, size_t length)
{
  /* 
    'm' and 'r' are mixing constants generated offline.  They're not
    really 'magic', they just happen to work well.
  */

  const unsigned int m= 0x5bd1e995;
  const unsigned int seed= (0xdeadbeef * length);
  const int r= 24;


  // Initialize the hash to a 'random' value

  unsigned int h= seed ^ length;

  // Mix 4 bytes at a time into the hash

  const unsigned char * data= (const unsigned char *)key;

  while(length >= 4)
  {
    unsigned int k = *(unsigned int *)data;

    k *= m; 
    k ^= k >> r; 
    k *= m; 

    h *= m; 
    h ^= k;

    data += 4;
    length -= 4;
  }

  // Handle the last few bytes of the input array

  switch(length)
  {
  case 3: h ^= data[2] << 16;
  case 2: h ^= data[1] << 8;
  case 1: h ^= data[0];
          h *= m;
  };

  /* 
    Do a few final mixes of the hash to ensure the last few bytes are
    well-incorporated.  
  */

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}
