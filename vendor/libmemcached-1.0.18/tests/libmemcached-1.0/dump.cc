/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
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

#include <cstdlib>
#include <climits>

#include <libtest/test.hpp>

#include <libmemcached-1.0/memcached.h>
#include <libmemcachedutil-1.0/util.h>

using namespace libtest;

#include "tests/libmemcached-1.0/dump.h"

static memcached_return_t callback_dump_counter(const memcached_st *,
                                                const char*, // key,
                                                size_t, // length,
                                                void *context)
{
  size_t *counter= (size_t *)context;

#if 0
  std::cerr.write(key, length);
  std::cerr << std::endl;
#endif

  *counter= *counter +1;

  return MEMCACHED_SUCCESS;
}

static memcached_return_t item_counter(const memcached_instance_st * ,
                                       const char *key, size_t key_length,
                                       const char *value, size_t, // value_length,
                                       void *context)
{
  if ((key_length == (sizeof("curr_items") -1)) and (strncmp("curr_items", key, (sizeof("curr_items") -1)) == 0))
  {
    uint64_t* counter= (uint64_t*)context;
    unsigned long number_value= strtoul(value, (char **)NULL, 10);
    if (number_value == ULONG_MAX)
    {
      return MEMCACHED_FAILURE;
    }
    *counter= *counter +number_value;
  }

  return MEMCACHED_SUCCESS;
}

#if 0
test_return_t memcached_dump_TEST(memcached_st *memc)
{
  test_skip(false, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  size_t count= 0;
  memcached_dump_fn callbacks[1];
  callbacks[0]= &callback_dump_counter;

  uint64_t counter= 0;
  test_compare_got(MEMCACHED_SUCCESS,
                   memcached_stat_execute(memc, NULL, item_counter, &counter),
                   memcached_last_error_message(memc));
  test_zero(counter);

  test_compare_got(MEMCACHED_SUCCESS, memcached_dump(memc, callbacks, &count, 1), memcached_last_error_message(memc));

  return TEST_SUCCESS;
}
#endif

#define memcached_dump_TEST2_COUNT 64
test_return_t memcached_dump_TEST2(memcached_st *memc)
{
  test_skip(false, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  /* The dump test relies on there being at least 32 items in memcached */
  for (uint32_t x= 0; x < memcached_dump_TEST2_COUNT; x++)
  {
    char key[1024];

    int length= snprintf(key, sizeof(key), "%s%u", __func__, x);

    test_true(length > 0);

    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, key, length,
                               NULL, 0, // Zero length values
                               time_t(0), uint32_t(0)));
  }
  memcached_quit(memc);

  uint64_t counter= 0;
  test_compare(MEMCACHED_SUCCESS,
               memcached_stat_execute(memc, NULL, item_counter, &counter));
  test_true(counter > 0);

  size_t count= 0;
  memcached_dump_fn callbacks[1];
  callbacks[0]= &callback_dump_counter;

  test_compare(MEMCACHED_SUCCESS,
               memcached_dump(memc, callbacks, &count, 1));

  test_true(count);

  return TEST_SUCCESS;
}
