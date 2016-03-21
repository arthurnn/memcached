/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Test memslap
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

static std::string executable;

static test_return_t quiet_test(void *)
{
  const char *args[]= { "--quiet", 0 };

  test_compare(EXIT_FAILURE, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t help_test(void *)
{
  const char *args[]= { "--help", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_concurrency_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, "--concurrency=10", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_concurrency_initial_load_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, "--concurrency=10", "--initial-load=1000", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_concurrency_initial_load_execute_number_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, "--concurrency=10", "--initial-load=1000", "--execute-number=10", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_concurrency_initial_load_execute_number_test_get_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, "--concurrency=10", "--initial-load=1000", "--execute-number=10", "--test=get", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_concurrency_initial_load_execute_number_test_set_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, "--concurrency=10", "--initial-load=1000", "--execute-number=10", "--test=set", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

static test_return_t server_concurrency_initial_load_execute_number_test_set_non_blocking_test(void *)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "--servers=localhost:%d", int(default_port()));
  const char *args[]= { buffer, "--concurrency=10", "--initial-load=1000", "--execute-number=10", "--test=set", "--non-blocking", 0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(executable, args, true));

  return TEST_SUCCESS;
}

test_st memslap_tests[] ={
  {"--quiet", true, quiet_test },
  {"--help", true, help_test },
  {"--server_test", true, server_test },
  {"--concurrency=10", true, server_concurrency_test },
  {"--initial-load=1000", true, server_concurrency_initial_load_test },
  {"--execute-number=10", true, server_concurrency_initial_load_execute_number_test },
  {"--test=get", true, server_concurrency_initial_load_execute_number_test_get_test },
  {"--test=set", true, server_concurrency_initial_load_execute_number_test_set_test },
  {"--test=set --non-blockin", true, server_concurrency_initial_load_execute_number_test_set_non_blocking_test },
  {0, 0, 0}
};

collection_st collection[] ={
  {"memslap", 0, 0, memslap_tests },
  {0, 0, 0, 0}
};

static void *world_create(server_startup_st& servers, test_return_t& error)
{
  if (libtest::has_memcached() == false)
  {
    error= TEST_SKIPPED;
    return NULL;
  }

  const char *argv[]= { "memslap", 0 };
  if (server_startup(servers, "memcached", libtest::default_port(), argv) == false)
  {
    error= TEST_FAILURE;
  }

  return &servers;
}


void get_world(libtest::Framework* world)
{
  executable= "./clients/memslap";
  world->collections(collection);
  world->create(world_create);
}

