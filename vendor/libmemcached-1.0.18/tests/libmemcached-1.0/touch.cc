/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached Client and Server 
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
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

using namespace libtest;

#include <libmemcached-1.0/memcached.h>
#include <libmemcachedutil-1.0/util.h>

#include "tests/touch.h"

static test_return_t pre_touch(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_version(memc));
  test_skip(true, libmemcached_util_version_check(memc, 1, 4, 15));

  return TEST_SUCCESS;
}

test_return_t test_memcached_touch(memcached_st *memc)
{
  test_skip(TEST_SUCCESS, pre_touch(memc));

  size_t len;
  uint32_t flags;
  memcached_return rc;

  test_null(memcached_get(memc, 
                          test_literal_param(__func__),
                          &len, &flags, &rc));
  test_zero(len);
  test_compare(MEMCACHED_NOTFOUND, rc);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_set(memc,
                             test_literal_param(__func__),
                             test_literal_param("touchval"),
                             2, 0));

  {
    char *value= memcached_get(memc, 
                               test_literal_param(__func__),
                               &len, &flags, &rc);
    test_compare(8U, test_literal_param_size("touchval"));
    test_true(value);
    test_strcmp(value, "touchval");
    test_compare(MEMCACHED_SUCCESS, rc);
    free(value);
  }

  rc= memcached_touch(memc, test_literal_param(__func__), 60 *60);
  ASSERT_EQ_(MEMCACHED_SUCCESS, rc, "%s", memcached_last_error_message(memc));

  rc= memcached_touch(memc, test_literal_param(__func__), 60 *60 *24 *60);
  ASSERT_EQ_(MEMCACHED_SUCCESS, rc, "%s", memcached_last_error_message(memc));

  rc= memcached_exist(memc, test_literal_param(__func__));
  ASSERT_EQ_(MEMCACHED_NOTFOUND, rc, "%s", memcached_last_error_message(memc));

  return TEST_SUCCESS;
}

test_return_t test_memcached_touch_by_key(memcached_st *memc)
{
  test_skip(TEST_SUCCESS, pre_touch(memc));

  size_t len;
  uint32_t flags;
  memcached_return rc;

  test_null(memcached_get_by_key(memc, 
                                 test_literal_param("grouping_key"),
                                 test_literal_param(__func__),
                                 &len, &flags, &rc));
  test_zero(len);
  test_compare(MEMCACHED_NOTFOUND, rc);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_set_by_key(memc,
                                    test_literal_param("grouping_key"),
                                    test_literal_param(__func__),
                                    test_literal_param("touchval"),
                                    2, 0));

  {
    char *value= memcached_get_by_key(memc, 
                                      test_literal_param("grouping_key"),
                                      test_literal_param(__func__),
                                      &len, &flags, &rc);
    test_compare(8U, test_literal_param_size("touchval"));
    test_true(value);
    test_strcmp(value, "touchval");
    test_compare(MEMCACHED_SUCCESS, rc);
    free(value);
  }

  rc= memcached_touch_by_key(memc,
                             test_literal_param("grouping_key"),
                             test_literal_param(__func__),
                             60 *60);
  ASSERT_EQ_(MEMCACHED_SUCCESS, rc, "%s", memcached_last_error_message(memc));

  rc= memcached_touch_by_key(memc,
                             test_literal_param("grouping_key"),
                             test_literal_param(__func__),
                             60 *60 *24 *60);
  ASSERT_EQ_(MEMCACHED_SUCCESS, rc, "%s", memcached_last_error_message(memc));

  rc= memcached_exist_by_key(memc, test_literal_param("grouping_key"),test_literal_param(__func__));
  ASSERT_EQ_(MEMCACHED_NOTFOUND, rc, "%s", memcached_last_error_message(memc));

  return TEST_SUCCESS;
}



