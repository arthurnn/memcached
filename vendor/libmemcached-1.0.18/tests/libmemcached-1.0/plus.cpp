/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker All rights reserved.
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

#include <string>
#include <iostream>

using namespace std;
using namespace memcache;
using namespace libtest;

static void populate_vector(vector<char> &vec, const string &str)
{
  vec.reserve(str.length());
  vec.assign(str.begin(), str.end());
}

static void copy_vec_to_string(vector<char> &vec, string &str)
{
  str.clear();
  if (not vec.empty())
  {
    str.assign(vec.begin(), vec.end());
  }
}

static test_return_t basic_test(memcached_st *memc)
{
  Memcache foo(memc);
  const string value_set("This is some data");
  std::vector<char> value;
  std::vector<char> test_value;

  populate_vector(value, value_set);

  test_true(foo.set("mine", value, 0, 0));
  test_true(foo.get("mine", test_value));

  test_compare(test_value.size(), value.size());
  test_memcmp(&test_value[0], &value[0], test_value.size());
  test_false(foo.set("", value, 0, 0));

  return TEST_SUCCESS;
}

static test_return_t increment_test(memcached_st *original)
{
  Memcache mcach(original);
  const string key("blah");
  const string inc_value("1");
  std::vector<char> inc_val;
  vector<char> ret_value;
  string ret_string;
  uint64_t int_inc_value;
  uint64_t int_ret_value;

  populate_vector(inc_val, inc_value);

  test_true(mcach.set(key, inc_val, 0, 0));

  test_true(mcach.get(key, ret_value));
  test_false(ret_value.empty());
  copy_vec_to_string(ret_value, ret_string);

  int_inc_value= uint64_t(atol(inc_value.c_str()));
  int_ret_value= uint64_t(atol(ret_string.c_str()));
  test_compare(int_inc_value, int_ret_value);

  test_true(mcach.increment(key, 1, &int_ret_value));
  test_compare(uint64_t(2), int_ret_value);

  test_true(mcach.increment(key, 1, &int_ret_value));
  test_compare(uint64_t(3), int_ret_value);

  test_true(mcach.increment(key, 5, &int_ret_value));
  test_compare(uint64_t(8), int_ret_value);

  return TEST_SUCCESS;
}

static test_return_t basic_master_key_test(memcached_st *original)
{
  Memcache foo(original);
  const string value_set("Data for server A");
  vector<char> value;
  vector<char> test_value;
  const string master_key_a("server-a");
  const string master_key_b("server-b");
  const string key("xyz");

  populate_vector(value, value_set);

  test_true(foo.setByKey(master_key_a, key, value, 0, 0));
  test_true(foo.getByKey(master_key_a, key, test_value));

  test_compare(value.size(), test_value.size());
  test_memcmp(&value[0], &test_value[0], value.size());

  test_value.clear();

#if 0
  test_false(foo.getByKey(master_key_b, key, test_value));
  test_zero(test_value.size());
#endif

  return TEST_SUCCESS;
}

static test_return_t mget_test(memcached_st *original)
{
  Memcache memc(original);
  memcached_return_t mc_rc;
  vector<string> keys;
  vector< vector<char> *> values;
  keys.reserve(3);
  keys.push_back("fudge");
  keys.push_back("son");
  keys.push_back("food");
  vector<char> val1;
  vector<char> val2;
  vector<char> val3;
  populate_vector(val1, "fudge");
  populate_vector(val2, "son");
  populate_vector(val3, "food");
  values.reserve(3);
  values.push_back(&val1);
  values.push_back(&val2);
  values.push_back(&val3);

  string return_key;
  vector<char> return_value;

  /* We need to empty the server before we continue the test */
  bool flush_res= memc.flush();
  if (flush_res == false)
  {
    std::string error_string;
    ASSERT_TRUE(memc.error(error_string));
    Error << error_string;
  }
  test_true(memc.flush());

  test_true(memc.mget(keys));

  test_compare(MEMCACHED_NOTFOUND, 
               memc.fetch(return_key, return_value));

  test_true(memc.setAll(keys, values, 50, 9));

  test_true(memc.mget(keys));
  size_t count= 0;
  while (memcached_success(mc_rc= memc.fetch(return_key, return_value)))
  {
    test_compare(return_key.length(), return_value.size());
    test_memcmp(&return_value[0], return_key.c_str(), return_value.size());
    count++;
  }
  test_compare(values.size(), count);

  return TEST_SUCCESS;
}

static test_return_t lp_1010899_TEST(void*)
{
  // Check to see everything is setup internally even when no initial hosts are
  // given.
  Memcache memc;

  test_false(memc.increment(__func__, 0, NULL));

  return TEST_SUCCESS;
}

static test_return_t lp_1010899_with_args_TEST(memcached_st *original)
{
  // Check to see everything is setup internally even when a host is specified
  // on creation.
  const memcached_instance_st* instance= memcached_server_instance_by_position(original, 0);
  Memcache memc(memcached_server_name(instance), memcached_server_port(instance));

  test_false(memc.increment(__func__, 0, NULL));
  test_true(memc.set(__func__, test_literal_param("12"), 0, 0)); 
  test_true(memc.increment(__func__, 3, NULL));

  std::vector<char> ret_val;
  test_true(memc.get(__func__, ret_val));

  test_strcmp(&ret_val[0], "15");

  return TEST_SUCCESS;
}

static test_return_t basic_behavior(memcached_st *original)
{
  Memcache memc(original);
  test_true(memc.setBehavior(MEMCACHED_BEHAVIOR_VERIFY_KEY, true));
  test_compare(true, memc.getBehavior(MEMCACHED_BEHAVIOR_VERIFY_KEY));

  return TEST_SUCCESS;
}

static test_return_t error_test(memcached_st *)
{
  Memcache memc("--server=localhost:178");
  std::vector<char> value;

  test_false(memc.set("key", value, time_t(0), uint32_t(0)));

  test_true(memc.error());

  return TEST_SUCCESS;
}

static test_return_t error_std_string_test(memcached_st *)
{
  Memcache memc("--server=localhost:178");
  std::vector<char> value;

  test_false(memc.set("key", value, time_t(0), uint32_t(0)));

  std::string error_message;
  test_true(memc.error(error_message));
  test_false(error_message.empty());

  return TEST_SUCCESS;
}

static test_return_t error_memcached_return_t_test(memcached_st *)
{
  Memcache memc("--server=localhost:178");
  std::vector<char> value;

  test_false(memc.set("key", value, time_t(0), uint32_t(0)));

  memcached_return_t rc;
  test_true(memc.error(rc));
  test_compare(MEMCACHED_CONNECTION_FAILURE, rc);

  return TEST_SUCCESS;
}

test_st error_tests[] ={
  { "error()", false, reinterpret_cast<test_callback_fn*>(error_test) },
  { "error(std::string&)", false, reinterpret_cast<test_callback_fn*>(error_std_string_test) },
  { "error(memcached_return_t&)", false, reinterpret_cast<test_callback_fn*>(error_memcached_return_t_test) },
  {0, 0, 0}
};

test_st tests[] ={
  { "basic", false,
    reinterpret_cast<test_callback_fn*>(basic_test) },
  { "basic_master_key", false,
    reinterpret_cast<test_callback_fn*>(basic_master_key_test) },
  { "increment_test", false,
    reinterpret_cast<test_callback_fn*>(increment_test) },
  { "mget", true,
    reinterpret_cast<test_callback_fn*>(mget_test) },
  { "basic_behavior", false,
    reinterpret_cast<test_callback_fn*>(basic_behavior) },
  {0, 0, 0}
};

test_st regression_TESTS[] ={
  { "lp:1010899 Memcache()", false, lp_1010899_TEST },
  { "lp:1010899 Memcache(localhost, port)", false,
    reinterpret_cast<test_callback_fn*>(lp_1010899_with_args_TEST) },
  {0, 0, 0}
};

collection_st collection[] ={
  {"block", 0, 0, tests},
  {"error()", 0, 0, error_tests},
  {"regression", 0, 0, regression_TESTS},
  {0, 0, 0, 0}
};

#define SERVERS_TO_CREATE 5

#define TEST_PORT_BASE MEMCACHED_DEFAULT_PORT +10
#include "tests/libmemcached_world.h"

void get_world(libtest::Framework* world)
{
  world->collections(collection);

  world->create((test_callback_create_fn*)world_create);
  world->destroy((test_callback_destroy_fn*)world_destroy);

  world->set_runner(new LibmemcachedRunner);
}
