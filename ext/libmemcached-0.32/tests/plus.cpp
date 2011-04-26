/*
  C++ interface test
*/
#include "libmemcached/memcached.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "server.h"

#include "test.h"

#include <string>

using namespace std;
using namespace memcache;

extern "C" {
   test_return basic_test(memcached_st *memc);
   test_return increment_test(memcached_st *memc);
   test_return basic_master_key_test(memcached_st *memc);
   test_return mget_result_function(memcached_st *memc);
   test_return mget_test(memcached_st *memc);
   memcached_return callback_counter(memcached_st *,
                                     memcached_result_st *, 
                                     void *context);
   void *world_create(void);
   void world_destroy(void *p);
}

static void populate_vector(vector<char> &vec, const string &str)
{
  vec.reserve(str.length());
  vec.assign(str.begin(), str.end());
}

static void copy_vec_to_string(vector<char> &vec, string &str)
{
  str.clear();
  if (! vec.empty())
  {
    str.assign(vec.begin(), vec.end());
  }
}

test_return basic_test(memcached_st *memc)
{
  Memcache foo(memc);
  const string value_set("This is some data");
  std::vector<char> value;
  std::vector<char> test_value;

  populate_vector(value, value_set);

  foo.set("mine", value, 0, 0);
  foo.get("mine", test_value);

  assert((memcmp(&test_value[0], &value[0], test_value.size()) == 0));

  return TEST_SUCCESS;
}

test_return increment_test(memcached_st *memc)
{
  Memcache mcach(memc);
  bool rc;
  const string key("blah");
  const string inc_value("1");
  std::vector<char> inc_val;
  vector<char> ret_value;
  string ret_string;
  uint64_t int_inc_value;
  uint64_t int_ret_value;

  populate_vector(inc_val, inc_value);

  rc= mcach.set(key, inc_val, 0, 0);
  if (rc == false)
  {
    return TEST_FAILURE;
  }
  mcach.get(key, ret_value);
  if (ret_value.empty())
  {
    return TEST_FAILURE;
  }
  copy_vec_to_string(ret_value, ret_string);

  int_inc_value= uint64_t(atol(inc_value.c_str()));
  int_ret_value= uint64_t(atol(ret_string.c_str()));
  assert(int_ret_value == int_inc_value); 

  rc= mcach.increment(key, 1, &int_ret_value);
  assert(rc == true);
  assert(int_ret_value == 2);

  rc= mcach.increment(key, 1, &int_ret_value);
  assert(rc == true);
  assert(int_ret_value == 3);

  rc= mcach.increment(key, 5, &int_ret_value);
  assert(rc == true);
  assert(int_ret_value == 8);

  return TEST_SUCCESS;
}

test_return basic_master_key_test(memcached_st *memc)
{
  Memcache foo(memc);
  const string value_set("Data for server A");
  vector<char> value;
  vector<char> test_value;
  const string master_key_a("server-a");
  const string master_key_b("server-b");
  const string key("xyz");

  populate_vector(value, value_set);

  foo.setByKey(master_key_a, key, value, 0, 0);
  foo.getByKey(master_key_a, key, test_value);

  assert((memcmp(&value[0], &test_value[0], value.size()) == 0));

  test_value.clear();

  foo.getByKey(master_key_b, key, test_value);
  assert((memcmp(&value[0], &test_value[0], value.size()) == 0));

  return TEST_SUCCESS;
}

/* Count the results */
memcached_return callback_counter(memcached_st *,
                                  memcached_result_st *, 
                                  void *context)
{
  unsigned int *counter= static_cast<unsigned int *>(context);

  *counter= *counter + 1;

  return MEMCACHED_SUCCESS;
}

test_return mget_result_function(memcached_st *memc)
{
  Memcache mc(memc);
  bool rc;
  string key1("fudge");
  string key2("son");
  string key3("food");
  vector<string> keys;
  vector< vector<char> *> values;
  vector<char> val1;
  vector<char> val2;
  vector<char> val3;
  populate_vector(val1, key1);
  populate_vector(val2, key2);
  populate_vector(val3, key3);
  keys.reserve(3);
  keys.push_back(key1);
  keys.push_back(key2);
  keys.push_back(key3);
  values.reserve(3);
  values.push_back(&val1);
  values.push_back(&val2);
  values.push_back(&val3);
  unsigned int counter;
  memcached_execute_function callbacks[1];

  /* We need to empty the server before we continue the test */
  rc= mc.flush(0);
  rc= mc.setAll(keys, values, 50, 9);
  assert(rc == true);

  rc= mc.mget(keys);
  assert(rc == true);

  callbacks[0]= &callback_counter;
  counter= 0;
  rc= mc.fetchExecute(callbacks, static_cast<void *>(&counter), 1); 

  assert(counter == 3);

  return TEST_SUCCESS;
}

test_return mget_test(memcached_st *memc)
{
  Memcache mc(memc);
  bool rc;
  memcached_return mc_rc;
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
  rc= mc.flush(0);
  assert(rc == true);

  rc= mc.mget(keys);
  assert(rc == true);

  while ((mc_rc= mc.fetch(return_key, return_value)) != MEMCACHED_END)
  {
    assert(return_value.size() != 0);
    return_value.clear();
  }
  assert(mc_rc == MEMCACHED_END);

  rc= mc.setAll(keys, values, 50, 9);
  assert(rc == true);

  rc= mc.mget(keys);
  assert(rc == true);

  while ((mc_rc= mc.fetch(return_key, return_value)) != MEMCACHED_END)
  {
    assert(return_key.length() == return_value.size());
    assert(!memcmp(&return_value[0], return_key.c_str(), return_value.size()));
  }

  return TEST_SUCCESS;
}

test_st tests[] ={
  { "basic", 0, basic_test },
  { "basic_master_key", 0, basic_master_key_test },
  { "increment_test", 0, increment_test },
  { "mget", 1, mget_test },
  { "mget_result_function", 1, mget_result_function },
  {0, 0, 0}
};

collection_st collection[] ={
  {"block", 0, 0, tests},
  {0, 0, 0, 0}
};

#define SERVERS_TO_CREATE 1

extern "C" void *world_create(void)
{
  server_startup_st *construct;

  construct= (server_startup_st *)malloc(sizeof(server_startup_st));
  memset(construct, 0, sizeof(server_startup_st));

  construct->count= SERVERS_TO_CREATE;
  server_startup(construct);

  return construct;
}

void world_destroy(void *p)
{
  server_startup_st *construct= static_cast<server_startup_st *>(p);
  memcached_server_st *servers=
    static_cast<memcached_server_st *>(construct->servers);
  memcached_server_list_free(servers);

  server_shutdown(construct);
  free(construct);
}

void get_world(world_st *world)
{
  world->collections= collection;
  world->create= world_create;
  world->destroy= world_destroy;
}
