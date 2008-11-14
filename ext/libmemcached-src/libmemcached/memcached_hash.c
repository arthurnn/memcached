#include "common.h"

/* Defines */
static uint64_t FNV_64_INIT= 0xcbf29ce484222325LL;
static uint64_t FNV_64_PRIME= 0x100000001b3LL;

static uint32_t FNV_32_INIT= 2166136261UL;
static uint32_t FNV_32_PRIME= 16777619;

/* Prototypes */
static uint32_t internal_generate_hash(const char *key, size_t key_length);
static uint32_t internal_generate_md5(const char *key, size_t key_length);

uint32_t generate_hash_value(const char *key, size_t key_length, memcached_hash hash_algorithm)
{
  uint32_t hash= 1; /* Just here to remove compile warning */
  uint32_t x= 0;

  switch (hash_algorithm)
  {
  case MEMCACHED_HASH_DEFAULT:
    hash= internal_generate_hash(key, key_length);
    break;
  case MEMCACHED_HASH_MD5:
    hash= internal_generate_md5(key, key_length);
    break;
  case MEMCACHED_HASH_CRC:
    hash= ((hash_crc32(key, key_length) >> 16) & 0x7fff);
    if (hash == 0)
      hash= 1;
    break;
    /* FNV hash'es lifted from Dustin Sallings work */
  case MEMCACHED_HASH_FNV1_64: 
    {
      /* Thanks to pierre@demartines.com for the pointer */
      uint64_t temp_hash;

      temp_hash= FNV_64_INIT;
      for (x= 0; x < key_length; x++) 
      {
        temp_hash *= FNV_64_PRIME;
        temp_hash ^= key[x];
      }
      hash= (uint32_t)temp_hash;
    }
    break;
  case MEMCACHED_HASH_FNV1A_64: 
    {
      hash= FNV_64_INIT;
      for (x= 0; x < key_length; x++) 
      {
        hash ^= key[x];
        hash *= FNV_64_PRIME;
      }
    }
    break;
  case MEMCACHED_HASH_FNV1_32: 
    {
      hash= FNV_32_INIT;
      for (x= 0; x < key_length; x++) 
      {
        hash *= FNV_32_PRIME;
        hash ^= key[x];
      }
    }
    break;
  case MEMCACHED_HASH_FNV1A_32: 
    {
      hash= FNV_32_INIT;
      for (x= 0; x < key_length; x++) 
      {
        hash ^= key[x];
        hash *= FNV_32_PRIME;
      }
    }
    break;
    case MEMCACHED_HASH_HSIEH:
    {
      hash= hsieh_hash(key, key_length);
      break;
    }
    case MEMCACHED_HASH_MURMUR:
    {
      hash= murmur_hash(key, key_length);
      break;
    }
    case MEMCACHED_HASH_JENKINS:
    {
      hash=jenkins_hash(key, key_length, 13);
      break;
    }
  }
  return hash;
}

uint32_t generate_hash(memcached_st *ptr, const char *key, size_t key_length)
{
  uint32_t hash= 1; /* Just here to remove compile warning */


  WATCHPOINT_ASSERT(ptr->number_of_hosts);

  if (ptr->number_of_hosts == 1)
    return 0;

  hash= generate_hash_value(key, key_length, ptr->hash);
  WATCHPOINT_ASSERT(hash);
  return hash;
}

static uint32_t dispatch_host(memcached_st *ptr, uint32_t hash)
{
  switch (ptr->distribution) 
  {
  case MEMCACHED_DISTRIBUTION_CONSISTENT:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA:
    {
      uint32_t num= ptr->continuum_points_counter;
      WATCHPOINT_ASSERT(ptr->continuum);

      hash= hash;
      memcached_continuum_item_st *begin, *end, *left, *right, *middle;
      begin= left= ptr->continuum;
      end= right= ptr->continuum + num;

      while (left < right)
      {
        middle= left + (right - left) / 2;
        if (middle->value < hash)
          left= middle + 1;
        else
          right= middle;
      }
      if (right > end)
        right= begin;
      return right->index;
    } 
    break;
  case MEMCACHED_DISTRIBUTION_MODULA:
    return hash % ptr->number_of_hosts;
  case MEMCACHED_DISTRIBUTION_RANDOM:
    return random() % ptr->number_of_hosts;
  default:
    WATCHPOINT_ASSERT(0); /* We have added a distribution without extending the logic */
    return hash % ptr->number_of_hosts;
  }

  WATCHPOINT_ASSERT(0); /* We should never reach here */
  return 0;
}

/* 
  One day make this public, and have it return the actual memcached_server_st 
  to the calling application.
*/
uint32_t memcached_generate_hash(memcached_st *ptr, const char *key, size_t key_length)
{
  uint32_t hash= 1; /* Just here to remove compile warning */

  WATCHPOINT_ASSERT(ptr->number_of_hosts);

  if (ptr->number_of_hosts == 1)
    return 0;

  if (ptr->flags & MEM_HASH_WITH_PREFIX_KEY)
  {
    int temp_len= ptr->prefix_key_length + key_length;
    char *temp= (char *)malloc(temp_len);
    strncpy(temp, ptr->prefix_key, ptr->prefix_key_length);
    strncpy(temp + ptr->prefix_key_length, key, key_length);
    hash= generate_hash(ptr, temp, temp_len);
    free(temp);
  }
  else
  {
    hash= generate_hash(ptr, key, key_length);
  }

  WATCHPOINT_ASSERT(hash);

  return dispatch_host(ptr, hash);
}

static uint32_t internal_generate_hash(const char *key, size_t key_length)
{
  const char *ptr= key;
  uint32_t value= 0;

  while (--key_length) 
  {
    value += *ptr++;
    value += (value << 10);
    value ^= (value >> 6);
  }
  value += (value << 3);
  value ^= (value >> 11);
  value += (value << 15); 

  return value == 0 ? 1 : value;
}

static uint32_t internal_generate_md5(const char *key, size_t key_length)
{
  unsigned char results[16];

  md5_signature((unsigned char*)key, (unsigned int)key_length, results);

  return ((uint32_t) (results[3] & 0xFF) << 24)
    | ((uint32_t) (results[2] & 0xFF) << 16)
    | ((uint32_t) (results[1] & 0xFF) << 8)
    | (results[0] & 0xFF);
}
