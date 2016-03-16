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

#include <libmemcachedutil-1.0/util.h>
#include <libmemcached/is.h>

#include <tests/libmemcached-1.0/generate.h>
#include <tests/libmemcached-1.0/fetch_all_results.h>
#include "tests/libmemcached-1.0/callback_counter.h"

#include "clients/generator.h"
#include "clients/execute.h"

#include "tests/memc.hpp"

#ifdef __APPLE__
# define GLOBAL_COUNT 3000
#else
# define GLOBAL_COUNT 10000
#endif

#define GLOBAL2_COUNT 100

using namespace libtest;

static pairs_st *global_pairs= NULL;
static const char *global_keys[GLOBAL_COUNT];
static size_t global_keys_length[GLOBAL_COUNT];
static size_t global_count= 0;

test_return_t cleanup_pairs(memcached_st*)
{
  pairs_free(global_pairs);
  global_pairs= NULL;

  return TEST_SUCCESS;
}

static test_return_t generate_pairs(memcached_st *)
{
  global_pairs= pairs_generate(GLOBAL_COUNT, 400);

  for (size_t x= 0; x < GLOBAL_COUNT; ++x)
  {
    global_keys[x]= global_pairs[x].key;
    global_keys_length[x]=  global_pairs[x].key_length;
  }

  return TEST_SUCCESS;
}

test_return_t generate_large_pairs(memcached_st *memc)
{
  global_pairs= pairs_generate(GLOBAL2_COUNT, MEMCACHED_MAX_BUFFER+10);

  for (size_t x= 0; x < GLOBAL2_COUNT; x++)
  {
    global_keys[x]= global_pairs[x].key;
    global_keys_length[x]=  global_pairs[x].key_length;
  }

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, true);
  global_count= execute_set(memc, global_pairs, (unsigned int)GLOBAL2_COUNT);

  ASSERT_TRUE(global_count > (GLOBAL2_COUNT / 2));

  return TEST_SUCCESS;
}

test_return_t generate_data(memcached_st *memc)
{
  test_compare(TEST_SUCCESS, generate_pairs(memc));

  global_count= execute_set(memc, global_pairs, (unsigned int)GLOBAL_COUNT);

  /* Possible false, positive, memcached may have ejected key/value based on
   * memory needs. */

  ASSERT_TRUE(global_count > (GLOBAL2_COUNT / 2));

  return TEST_SUCCESS;
}

test_return_t generate_data_with_stats(memcached_st *memc)
{
  test_compare(TEST_SUCCESS, generate_pairs(memc));

  global_count= execute_set(memc, global_pairs, (unsigned int)GLOBAL2_COUNT);

  ASSERT_EQ(global_count, GLOBAL2_COUNT);

  // @todo hosts used size stats
  memcached_return_t rc;
  memcached_stat_st *stat_p= memcached_stat(memc, NULL, &rc);
  test_true(stat_p);

  for (uint32_t host_index= 0; host_index < memcached_server_count(memc); host_index++)
  {
    /* This test was changes so that "make test" would work properlly */
    if (DEBUG)
    {
      const memcached_instance_st * instance=
        memcached_server_instance_by_position(memc, host_index);

      printf("\nserver %u|%s|%u bytes: %llu\n",
             host_index,
             memcached_server_name(instance),
             memcached_server_port(instance),
             (unsigned long long)(stat_p + host_index)->bytes);
    }
    test_true((unsigned long long)(stat_p + host_index)->bytes);
  }

  memcached_stat_free(NULL, stat_p);

  return TEST_SUCCESS;
}

test_return_t generate_buffer_data(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, true);
  generate_data(memc);

  return TEST_SUCCESS;
}

test_return_t get_read_count(memcached_st *memc)
{
  memcached_st *memc_clone= memcached_clone(NULL, memc);
  test_true(memc_clone);

  memcached_server_add_with_weight(memc_clone, "localhost", 6666, 0);

  {
    char *return_value;
    size_t return_value_length;
    uint32_t flags;
    uint32_t count;

    for (size_t x= count= 0; x < global_count; ++x)
    {
      memcached_return_t rc;
      return_value= memcached_get(memc_clone, global_keys[x], global_keys_length[x],
                                  &return_value_length, &flags, &rc);
      if (rc == MEMCACHED_SUCCESS)
      {
        count++;
        if (return_value)
        {
          free(return_value);
        }
      }
    }
  }

  memcached_free(memc_clone);

  return TEST_SUCCESS;
}

test_return_t get_read(memcached_st *memc)
{
  test::Memc clone(memc);
  size_t keys_returned= 0;
  for (size_t x= 0; x < global_count; ++x)
  {
    size_t return_value_length;
    uint32_t flags;
    memcached_return_t rc;
    char *return_value= memcached_get(&clone, global_keys[x], global_keys_length[x],
                                      &return_value_length, &flags, &rc);
    /*
      test_true(return_value);
      test_compare(MEMCACHED_SUCCESS, rc);
    */
    if (rc == MEMCACHED_SUCCESS && return_value)
    {
      keys_returned++;
      free(return_value);
    }
  }
  /*
    Possible false, positive, memcached may have ejected key/value based on memory needs.
  */
  test_true(keys_returned > (global_count / 2));

  return TEST_SUCCESS;
}

test_return_t mget_read(memcached_st *memc)
{

  test_skip(true, bool(libmemcached_util_version_check(memc, 1, 4, 4)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, global_keys, global_keys_length, global_count));

  // Go fetch the keys and test to see if all of them were returned
  {
    unsigned int keys_returned;
    test_compare(TEST_SUCCESS, fetch_all_results(memc, keys_returned));
    test_true(keys_returned > (global_count / 2));
  }

  return TEST_SUCCESS;
}

test_return_t mget_read_result(memcached_st *memc)
{

  test_skip(true, bool(libmemcached_util_version_check(memc, 1, 4, 4)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, global_keys, global_keys_length, global_count));

  /* Turn this into a help function */
  {
    memcached_result_st results_obj;
    memcached_result_st *results= memcached_result_create(memc, &results_obj);
    test_true(results);

    memcached_return_t rc;
    while ((results= memcached_fetch_result(memc, &results_obj, &rc)))
    {
      if (rc == MEMCACHED_IN_PROGRESS)
      {
        continue;
      }

      test_true(results);
      test_compare(MEMCACHED_SUCCESS, rc);
    }
    test_compare(MEMCACHED_END, rc);

    memcached_result_free(&results_obj);
  }

  return TEST_SUCCESS;
}

test_return_t mget_read_partial_result(memcached_st *memc)
{

  test_skip(true, bool(libmemcached_util_version_check(memc, 1, 4, 4)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, global_keys, global_keys_length, global_count));

  // We will scan for just one key
  {
    memcached_result_st results_obj;
    memcached_result_st *results= memcached_result_create(memc, &results_obj);

    memcached_return_t rc;
    results= memcached_fetch_result(memc, results, &rc);
    test_true(results);
    test_compare(MEMCACHED_SUCCESS, rc);

    memcached_result_free(&results_obj);
  }

  // We already have a read happening, lets start up another one.
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, global_keys, global_keys_length, global_count));
  {
    memcached_result_st results_obj;
    memcached_result_st *results= memcached_result_create(memc, &results_obj);
    test_true(results);
    test_false(memcached_is_allocated(results));

    memcached_return_t rc;
    while ((results= memcached_fetch_result(memc, &results_obj, &rc)))
    {
      test_true(results);
      test_compare(MEMCACHED_SUCCESS, rc);
    }
    test_compare(MEMCACHED_END, rc);

    memcached_result_free(&results_obj);
  }

  return TEST_SUCCESS;
}

test_return_t mget_read_function(memcached_st *memc)
{
  test_skip(true, bool(libmemcached_util_version_check(memc, 1, 4, 4)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, global_keys, global_keys_length, global_count));

  memcached_execute_fn callbacks[]= { &callback_counter };
  size_t counter= 0;
  test_compare(MEMCACHED_SUCCESS, 
               memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));

  return TEST_SUCCESS;
}

test_return_t delete_generate(memcached_st *memc)
{
  size_t total= 0;
  for (size_t x= 0; x < global_count; x++)
  {
    if (memcached_success(memcached_delete(memc, global_keys[x], global_keys_length[x], (time_t)0)))
    {
      total++;
    }
  }

  /*
    Possible false, positive, memcached may have ejected key/value based on memory needs.
  */
  ASSERT_TRUE(total);

  return TEST_SUCCESS;
}

test_return_t delete_buffer_generate(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, true);

  size_t total= 0;
  for (size_t x= 0; x < global_count; x++)
  {
    if (memcached_success(memcached_delete(memc, global_keys[x], global_keys_length[x], (time_t)0)))
    {
      total++;
    }
  }

  ASSERT_TRUE(total);

  return TEST_SUCCESS;
}

test_return_t mget_read_internal_result(memcached_st *memc)
{

  test_skip(true, bool(libmemcached_util_version_check(memc, 1, 4, 4)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, global_keys, global_keys_length, global_count));
  {
    memcached_result_st *results= NULL;
    memcached_return_t rc;
    while ((results= memcached_fetch_result(memc, results, &rc)))
    {
      test_true(results);
      test_compare(MEMCACHED_SUCCESS, rc);
    }
    test_compare(MEMCACHED_END, rc);

    memcached_result_free(results);
  }

  return TEST_SUCCESS;
}

