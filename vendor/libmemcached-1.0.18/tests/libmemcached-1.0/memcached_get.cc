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
#include <libtest/test.hpp>

/*
  Test cases
*/

#include <libmemcached-1.0/memcached.h>
#include "tests/libmemcached-1.0/memcached_get.h"
#include "tests/libmemcached-1.0/setup_and_teardowns.h"

test_return_t get_test(memcached_st *memc)
{
  uint64_t query_id= memcached_query_id(memc);
  memcached_return_t rc= memcached_delete(memc,
                                          test_literal_param(__func__),
                                          time_t(0));
  test_true(rc == MEMCACHED_BUFFERED or rc == MEMCACHED_NOTFOUND);
  test_compare(query_id +1, memcached_query_id(memc));

  size_t string_length;
  uint32_t flags;
  char *string= memcached_get(memc,
                        test_literal_param(__func__),
                        &string_length, &flags, &rc);

  test_compare(MEMCACHED_NOTFOUND, rc);
  test_false(string_length);
  test_false(string);

  return TEST_SUCCESS;
}

test_return_t get_test2(memcached_st *memc)
{
  const char *value= "when we sanitize";

  uint64_t query_id= memcached_query_id(memc);
  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             value, strlen(value),
                             time_t(0), uint32_t(0)));
  test_compare(query_id +1, memcached_query_id(memc));

  query_id= memcached_query_id(memc);
  test_true(query_id);

  uint32_t flags;
  size_t string_length;
  memcached_return_t rc;
  char *string= memcached_get(memc,
                              test_literal_param(__func__),
                              &string_length, &flags, &rc);
  test_compare(query_id +1, memcached_query_id(memc));

  test_compare(MEMCACHED_SUCCESS, rc);
  test_compare(MEMCACHED_SUCCESS, memcached_last_error(memc));
  test_true(string);
  test_compare(strlen(value), string_length);
  test_memcmp(string, value, string_length);

  free(string);

  return TEST_SUCCESS;
}

test_return_t get_test3(memcached_st *memc)
{
  size_t value_length= 8191;

  libtest::vchar_t value;
  value.reserve(value_length);
  for (uint32_t x= 0; x < value_length; x++)
  {
    value.push_back(char(x % 127));
  }

  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             &value[0], value.size(),
                             time_t(0), uint32_t(0)));

  size_t string_length;
  uint32_t flags;
  memcached_return_t rc;
  char *string= memcached_get(memc,
                              test_literal_param(__func__),
                              &string_length, &flags, &rc);

  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(string);
  test_compare(value.size(), string_length);
  test_memcmp(string, &value[0], string_length);

  free(string);

  return TEST_SUCCESS;
}

test_return_t get_test4(memcached_st *memc)
{
  size_t value_length= 8191;

  libtest::vchar_t value;
  value.reserve(value_length);
  for (uint32_t x= 0; x < value_length; x++)
  {
    value.push_back(char(x % 127));
  }

  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             &value[0], value.size(),
                             time_t(0), uint32_t(0)));

  for (uint32_t x= 0; x < 10; x++)
  {
    uint32_t flags;
    size_t string_length;
    memcached_return_t rc;
    char *string= memcached_get(memc,
                                test_literal_param(__func__),
                                &string_length, &flags, &rc);

    test_compare(MEMCACHED_SUCCESS, rc);
    test_true(string);
    test_compare(value.size(), string_length);
    test_memcmp(string, &value[0], string_length);
    free(string);
  }

  return TEST_SUCCESS;
}

/*
 * This test verifies that memcached_read_one_response doesn't try to
 * dereference a NIL-pointer if you issue a multi-get and don't read out all
 * responses before you execute a storage command.
 */
test_return_t get_test5(memcached_st *memc)
{
  /*
  ** Request the same key twice, to ensure that we hash to the same server
  ** (so that we have multiple response values queued up) ;-)
  */
  const char *keys[]= { "key", "key" };
  size_t lengths[]= { 3, 3 };
  uint32_t flags;
  size_t rlen;

  test_compare(return_value_based_on_buffering(memc),
                    memcached_set(memc, keys[0], lengths[0],
                                  keys[0], lengths[0],
                                  time_t(0), uint32_t(0)));
  test_compare(MEMCACHED_SUCCESS, memcached_mget(memc, keys, lengths, test_array_length(keys)));

  memcached_result_st results_obj;
  memcached_result_st *results= memcached_result_create(memc, &results_obj);
  test_true(results);

  memcached_return_t rc;
  results= memcached_fetch_result(memc, &results_obj, &rc);
  test_true(results);

  memcached_result_free(&results_obj);

  /* Don't read out the second result, but issue a set instead.. */
  test_compare(MEMCACHED_SUCCESS, memcached_set(memc, keys[0], lengths[0], keys[0], lengths[0], 0, 0));

  char *val= memcached_get_by_key(memc, keys[0], lengths[0], "yek", 3,
                                  &rlen, &flags, &rc);
  test_false(val);
  test_compare(MEMCACHED_NOTFOUND, rc);
  val= memcached_get(memc, keys[0], lengths[0], &rlen, &flags, &rc);
  test_true(val);
  test_compare(MEMCACHED_SUCCESS, rc);
  free(val);

  return TEST_SUCCESS;
}
