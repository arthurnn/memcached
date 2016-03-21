/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached Client and Server 
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <mem_config.h>
#include <libtest/test.hpp>

#include "tests/libmemcached-1.0/haldenbrand.h"
#include "tests/libmemcached-1.0/fetch_all_results.h"

/* Test case provided by Cal Haldenbrand */
#define HALDENBRAND_KEY_COUNT 3000U // * 1024576
#define HALDENBRAND_FLAG_KEY 99 // * 1024576

test_return_t haldenbrand_TEST1(memcached_st *memc)
{
  /* We just keep looking at the same values over and over */
  srandom(10);

  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, true));
  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, true));


  /* add key */
  unsigned long long total= 0;
  for (uint32_t x= 0 ; total < 20 * 1024576 ; x++ )
  {
    uint32_t size= (uint32_t)(rand() % ( 5 * 1024 ) ) + 400;
    char randomstuff[6 * 1024];
    memset(randomstuff, 0, 6 * 1024);
    test_true(size < 6 * 1024); /* Being safe here */

    for (uint32_t j= 0 ; j < size ;j++)
    {
      randomstuff[j] = (signed char) ((rand() % 26) + 97);
    }

    total+= size;
    char key[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
    int key_length= snprintf(key, sizeof(key), "%u", x);
    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, key, key_length,
                               randomstuff, strlen(randomstuff),
                               time_t(0), HALDENBRAND_FLAG_KEY));
  }
  test_true(total > HALDENBRAND_KEY_COUNT);

  return TEST_SUCCESS;
}

/* Test case provided by Cal Haldenbrand */
test_return_t haldenbrand_TEST2(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, 
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, true));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, true));

#if 0
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE, 20 * 1024576));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE, 20 * 1024576));
  getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE);
  getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE);

  for (x= 0, errors= 0; total < 20 * 1024576 ; x++);
#endif

  size_t total_value_length= 0;
  for (uint32_t x= 0, errors= 0; total_value_length < 24576 ; x++)
  {
    uint32_t flags= 0;
    size_t val_len= 0;

    char key[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
    int key_length= snprintf(key, sizeof(key), "%u", x);

    memcached_return_t rc;
    char *getval= memcached_get(memc, key, key_length, &val_len, &flags, &rc);
    if (memcached_failed(rc))
    {
      if (rc == MEMCACHED_NOTFOUND)
      {
        errors++;
      }
      else
      {
        test_true(rc);
      }

      continue;
    }
    test_compare(uint32_t(HALDENBRAND_FLAG_KEY), flags);
    test_true(getval);

    total_value_length+= val_len;
    errors= 0;
    ::free(getval);
  }

  return TEST_SUCCESS;
}

/* Do a large mget() over all the keys we think exist */
test_return_t haldenbrand_TEST3(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, true));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, true));

#ifdef NOT_YET
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE, 20 * 1024576);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE, 20 * 1024576);
  getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE);
  getter = memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE);
#endif

  std::vector<size_t> key_lengths;
  key_lengths.resize(HALDENBRAND_KEY_COUNT);
  std::vector<char *> keys;
  keys.resize(key_lengths.size());
  for (uint32_t x= 0; x < key_lengths.size(); x++)
  {
    char key[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
    int key_length= snprintf(key, sizeof(key), "%u", x);
    test_true(key_length > 0 and key_length < MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1);
    keys[x]= strdup(key);
    key_lengths[x]= key_length;
  }

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, &keys[0], &key_lengths[0], key_lengths.size()));

  unsigned int keys_returned;
  test_compare(TEST_SUCCESS, fetch_all_results(memc, keys_returned));
  test_compare(HALDENBRAND_KEY_COUNT, keys_returned);

  for (libtest::vchar_ptr_t::iterator iter= keys.begin();
       iter != keys.end();
       iter++)
  {
    ::free(*iter);
  }


  return TEST_SUCCESS;
}

