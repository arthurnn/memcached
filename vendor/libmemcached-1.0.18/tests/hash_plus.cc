/*
  C++ to libhashkit
*/

#include <mem_config.h>

#include <libtest/test.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <libhashkit-1.0/hashkit.hpp>

using namespace libtest;

#include "tests/hash_results.h"

static test_return_t exists_test(void *)
{
  Hashkit hashk;
  (void)hashk;

  return TEST_SUCCESS;
}

static test_return_t new_test(void *)
{
  Hashkit *hashk= new Hashkit;

  (void)hashk;

  delete hashk;

  return TEST_SUCCESS;
}

static test_return_t copy_test(void *)
{
  Hashkit *hashk= new Hashkit;
  Hashkit *copy(hashk);

  (void)copy;

  delete hashk;

  return TEST_SUCCESS;
}

static test_return_t assign_test(void *)
{
  Hashkit hashk;
  Hashkit copy;

  copy= hashk;

  (void)copy;

  return TEST_SUCCESS;
}

static test_return_t digest_test(void *)
{
  Hashkit hashk;
  test_true(hashk.digest("Foo", sizeof("Foo")));

  return TEST_SUCCESS;
}

static test_return_t set_function_test(void *)
{
  Hashkit hashk;
  hashkit_hash_algorithm_t algo_list[]= { 
    HASHKIT_HASH_DEFAULT,
    HASHKIT_HASH_MD5,
    HASHKIT_HASH_CRC,
    HASHKIT_HASH_FNV1_64,
    HASHKIT_HASH_FNV1A_64,
    HASHKIT_HASH_FNV1_32,
    HASHKIT_HASH_FNV1A_32,
    HASHKIT_HASH_MURMUR,
    HASHKIT_HASH_JENKINS,
    HASHKIT_HASH_MAX
  };


  for (hashkit_hash_algorithm_t *algo= algo_list; *algo != HASHKIT_HASH_MAX; algo++)
  {
    hashkit_return_t rc= hashk.set_function(*algo);

    if (rc == HASHKIT_INVALID_ARGUMENT)
    {
      continue;
    }

    test_compare(HASHKIT_SUCCESS, rc);

    uint32_t *list;
    switch (*algo)
    {
    case HASHKIT_HASH_DEFAULT:
      list= one_at_a_time_values;
      break;

    case HASHKIT_HASH_MD5:
      list= md5_values;
      break;

    case HASHKIT_HASH_CRC:
      list= crc_values;
      break;

    case HASHKIT_HASH_FNV1_64:
      list= fnv1_64_values;
      break;

    case HASHKIT_HASH_FNV1A_64:
      list= fnv1a_64_values;
      break;

    case HASHKIT_HASH_FNV1_32:
      list= fnv1_32_values;
      break;

    case HASHKIT_HASH_FNV1A_32:
      list= fnv1a_32_values;
      break;

    case HASHKIT_HASH_HSIEH:
      list= hsieh_values;
      break;

    case HASHKIT_HASH_MURMUR3:
#ifdef WORDS_BIGENDIAN
      continue;
#endif
      list= murmur3_values;
      break;
    case HASHKIT_HASH_MURMUR:
#ifdef WORDS_BIGENDIAN
      continue;
#endif
      list= murmur_values;
      break;

    case HASHKIT_HASH_JENKINS:
      list= jenkins_values;
      break;

    case HASHKIT_HASH_CUSTOM:
    case HASHKIT_HASH_MAX:
    default:
      list= NULL;
      test_fail("We ended up on a non-existent hash");
    }

    // Now we make sure we did set the hash correctly.
    uint32_t x;
    const char **ptr;
    for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
    {
      uint32_t hash_val;

      hash_val= hashk.digest(*ptr, strlen(*ptr));
      char buffer[1024];
      snprintf(buffer, sizeof(buffer), "%lu %lus %s", (unsigned long)list[x], (unsigned long)hash_val, libhashkit_string_hash(*algo));
      test_compare(list[x], hash_val);
    }
  }

  return TEST_SUCCESS;
}

static test_return_t set_distribution_function_test(void *)
{
  Hashkit hashk;
  hashkit_return_t rc;

  rc= hashk.set_distribution_function(HASHKIT_HASH_CUSTOM);
  test_true(rc == HASHKIT_FAILURE or rc == HASHKIT_INVALID_ARGUMENT);

  test_compare(HASHKIT_SUCCESS,
               hashk.set_distribution_function(HASHKIT_HASH_JENKINS));

  return TEST_SUCCESS;
}

static test_return_t compare_function_test(void *)
{
  Hashkit a, b;

  b= a;
  
  test_true(a == b);

  b.set_function(HASHKIT_HASH_MURMUR);

  test_false(a == b);
  test_true(b == b);
  test_true(a == a);

  return TEST_SUCCESS;
}

test_st basic[] ={
  { "exists", 0, reinterpret_cast<test_callback_fn*>(exists_test) },
  { "new", 0, reinterpret_cast<test_callback_fn*>(new_test) },
  { "copy", 0, reinterpret_cast<test_callback_fn*>(copy_test) },
  { "assign", 0, reinterpret_cast<test_callback_fn*>(assign_test) },
  { "digest", 0, reinterpret_cast<test_callback_fn*>(digest_test) },
  { "set_function", 0, reinterpret_cast<test_callback_fn*>(set_function_test) },
  { "set_distribution_function", 0, reinterpret_cast<test_callback_fn*>(set_distribution_function_test) },
  { "compare", 0, reinterpret_cast<test_callback_fn*>(compare_function_test) },
  { 0, 0, 0}
};

collection_st collection[] ={
  {"basic", 0, 0, basic},
  {0, 0, 0, 0}
};

void get_world(libtest::Framework* world)
{
  world->collections(collection);
}
