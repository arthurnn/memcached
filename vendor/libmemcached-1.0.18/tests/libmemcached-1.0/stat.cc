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

#include "tests/libmemcached-1.0/stat.h"

static memcached_return_t item_counter(const memcached_instance_st * ,
                                       const char *key, size_t key_length,
                                       const char *value, size_t, // value_length,
                                       void *context)
{
  if ((key_length == (sizeof("curr_items") -1)) and (strncmp("curr_items", key, (sizeof("curr_items") -1)) == 0))
  {
    uint64_t* counter= (uint64_t*)context;
    unsigned long number_value= strtoul(value, (char **)NULL, 10);
    ASSERT_NEQ(number_value, ULONG_MAX);
    *counter= *counter +number_value;
  }

  return MEMCACHED_SUCCESS;
}

test_return_t memcached_stat_TEST(memcached_st *memc)
{
  uint64_t counter= 0;
  test_compare(MEMCACHED_INVALID_ARGUMENTS,
               memcached_stat_execute(memc, "BAD_ARG_VALUE", item_counter, &counter));

  return TEST_SUCCESS;
}

#define memcached_dump_TEST2_COUNT 64
test_return_t memcached_stat_TEST2(memcached_st *memc)
{
  test_skip(false, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  /* The dump test relies on there being at least 32 items in memcached */
  for (uint32_t x= 0; x < memcached_dump_TEST2_COUNT; x++)
  {
    char key[1024];
    int length= snprintf(key, sizeof(key), "%s%u", __func__, x);

    ASSERT_TRUE(length > 0);

    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, key, length,
                               NULL, 0, // Zero length values
                               time_t(0), uint32_t(0)));
  }
  memcached_quit(memc);

  uint64_t counter= 0;
  ASSERT_EQ(MEMCACHED_SUCCESS,
            memcached_stat_execute(memc, NULL, item_counter, &counter));
  ASSERT_TRUE(counter);

  return TEST_SUCCESS;
}
