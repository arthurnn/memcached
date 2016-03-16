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

/*
  C++ interface test
*/
#include <libmemcached-1.0/memcached.hpp>
#include <libtest/test.hpp>

using namespace libtest;

static test_return_t memcached_NULL_string_TEST(void*)
{
  test_null(memcached(NULL, 75));
  return TEST_SUCCESS;
}

static test_return_t memcached_zero_string_length_TEST(void*)
{
  test_null(memcached("value", 0));
  return TEST_SUCCESS;
}

static test_return_t putenv_localhost_quoted_TEST(void*)
{
  test_zero(setenv("LIBMEMCACHED", "\"--server=localhost\"", 1));
  test_null(memcached(NULL, 0));

  return TEST_SUCCESS;
}

static test_return_t putenv_NULL_TEST(void*)
{
  test_zero(setenv("LIBMEMCACHED", "", 1));
  memcached_st *memc= memcached(NULL, 0);
  test_true(memc);

  memcached_free(memc);

  return TEST_SUCCESS;
}

static test_return_t putenv_localhost_TEST(void*)
{
  test_zero(setenv("LIBMEMCACHED", "--server=localhost", 1));
  memcached_st *memc= memcached(NULL, 0);
  test_true(memc);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_st memcached_TESTS[] ={
  {"memcached(NULL, 75)", false, (test_callback_fn*)memcached_NULL_string_TEST },
  {"memcached(\"value\", 0)", false, (test_callback_fn*)memcached_zero_string_length_TEST },
  {"putenv(LIBMEMCACHED=--server=localhost)", false, (test_callback_fn*)putenv_localhost_TEST },
  {"putenv(LIBMEMCACHED)", false, (test_callback_fn*)putenv_NULL_TEST },
  {"putenv(LIBMEMCACHED=--server=\"localhost\")", false, (test_callback_fn*)putenv_localhost_quoted_TEST },
  {0, 0, 0}
};

collection_st collection[] ={
  {"memcached()", 0, 0, memcached_TESTS},
  {0, 0, 0, 0}
};

void get_world(libtest::Framework* world)
{
  world->collections(collection);
}

