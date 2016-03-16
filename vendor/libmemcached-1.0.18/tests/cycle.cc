/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Cycle the Gearmand server
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
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


/*
  Test that we are cycling the servers we are creating during testing.
*/

#include <mem_config.h>
#include <libtest/test.hpp>

using namespace libtest;
#include <libmemcached-1.0/memcached.h>

static test_return_t server_startup_single_TEST(void *obj)
{
  server_startup_st *servers= (server_startup_st*)obj;
  test_compare(true, server_startup(*servers, "memcached", libtest::get_free_port(), NULL));
  test_compare(true, servers->shutdown());


  return TEST_SUCCESS;
}

static test_return_t server_startup_multiple_TEST(void *obj)
{
  test_skip(true, jenkins_is_caller());

  server_startup_st *servers= (server_startup_st*)obj;
  for (size_t x= 0; x < 10; ++x)
  {
    test_compare(true, server_startup(*servers, "memcached", libtest::get_free_port(), NULL));
  }
  test_compare(true, servers->shutdown());

  return TEST_SUCCESS;
}

static test_return_t shutdown_and_remove_TEST(void *obj)
{
  server_startup_st *servers= (server_startup_st*)obj;
  servers->clear();

  return TEST_SUCCESS;
}

test_st server_startup_TESTS[] ={
  {"server_startup(1)", false, (test_callback_fn*)server_startup_single_TEST },
  {"server_startup(10)", false, (test_callback_fn*)server_startup_multiple_TEST },
  {"shutdown_and_remove()", false, (test_callback_fn*)shutdown_and_remove_TEST },
  {"server_startup(10)", false, (test_callback_fn*)server_startup_multiple_TEST },
  {0, 0, 0}
};

#if 0
static test_return_t collection_INIT(void *object)
{
  server_startup_st *servers= (server_startup_st*)object;
  test_zero(servers->count());
  test_compare(true, server_startup(*servers, "memcached", libtest::default_port(), 0, NULL));

  return TEST_SUCCESS;
}
#endif

static test_return_t validate_sanity_INIT(void *object)
{
  server_startup_st *servers= (server_startup_st*)object;

  test_zero(servers->count());

  return TEST_SUCCESS;
}

static test_return_t collection_FINAL(void *object)
{
  server_startup_st *servers= (server_startup_st*)object;
  servers->clear();

  return TEST_SUCCESS;
}

collection_st collection[] ={
  {"server_startup()", validate_sanity_INIT, collection_FINAL, server_startup_TESTS },
  {0, 0, 0, 0}
};

static void *world_create(server_startup_st& servers, test_return_t& error)
{
  if (jenkins_is_caller())
  {
    error= TEST_SKIPPED;
    return NULL;
  }

  if (libtest::has_memcached() == false)
  {
    error= TEST_SKIPPED;
    return NULL;
  }

  return &servers;
}

void get_world(libtest::Framework* world)
{
  world->collections(collection);
  world->create(world_create);
}

