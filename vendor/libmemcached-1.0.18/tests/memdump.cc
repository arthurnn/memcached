/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Test memdump
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
#include <libmemcached-1.0/memcached.h>

using namespace libtest;

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

static std::string executable("./clients/memdump");

static test_return_t help_test(void *)
{
  const char *args[]= { "--help", "--quiet", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, 0 };

  test_true(exec_cmdline(executable, args, true) <= EXIT_FAILURE);

  return TEST_SUCCESS;
}

static test_return_t FOUND_test(void *)
{
  char buffer[1024];
  int length= snprintf(buffer, sizeof(buffer), "--server=localhost:%d", int(default_port()));
  const char *args[]= { buffer, 0 };

  memcached_st *memc= memcached(buffer, length);
  test_true(memc);

  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc, test_literal_param("foo"), 0, 0, 0, 0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc, test_literal_param("foo2"), 0, 0, 0, 0));

  memcached_return_t rc;
  test_null(memcached_get(memc, test_literal_param("foo"), 0, 0, &rc));
  test_compare(MEMCACHED_SUCCESS, rc);

  length= snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  test_true(exec_cmdline(executable, args, true) <= EXIT_FAILURE);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_st memdump_tests[] ={
  {"--help", true, help_test },
  {"--server", true, server_test },
  {"FOUND", true, FOUND_test },
  {0, 0, 0}
};

collection_st collection[] ={
  {"memdump", 0, 0, memdump_tests },
  {0, 0, 0, 0}
};

static void *world_create(server_startup_st& servers, test_return_t&)
{
  SKIP_UNLESS(libtest::has_memcached());

  ASSERT_TRUE(server_startup(servers, "memcached", libtest::default_port(), NULL));

  return &servers;
}


void get_world(libtest::Framework* world)
{
  world->collections(collection);
  world->create(world_create);
}

