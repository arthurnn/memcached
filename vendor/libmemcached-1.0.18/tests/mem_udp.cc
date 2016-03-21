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


/*
  Sample test application.
*/

#include <mem_config.h>
#include <libtest/test.hpp>

using namespace libtest;

#include <libmemcached-1.0/memcached.h>
#include <libmemcached/server_instance.h>
#include <libmemcached/io.h>
#include <libmemcached/udp.hpp>
#include <libmemcachedutil-1.0/util.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <libtest/server.h>

#include "libmemcached/instance.hpp"

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

/**
  @note This should be testing to see if the server really supports the binary protocol.
*/
static test_return_t pre_binary(memcached_st *memc)
{
  memcached_st *memc_clone= memcached_clone(NULL, memc);
  test_true(memc_clone);

  // The memcached_version needs to be done on a clone, because the server
  // will not toggle protocol on an connection.
  memcached_version(memc_clone);

  test_compare(MEMCACHED_SUCCESS, memcached_version(memc));
  test_compare(true, libmemcached_util_version_check(memc, 1, 2, 1));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, true));
  test_compare(true, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  memcached_free(memc_clone);

  return TEST_SUCCESS;
}

typedef std::vector<uint16_t> Expected;

static void increment_request_id(uint16_t *id)
{
  (*id)++;
  if ((*id & UDP_REQUEST_ID_THREAD_MASK) != 0)
  {
    *id= 0;
  }
}

static void get_udp_request_ids(memcached_st *memc, Expected &ids)
{
  for (uint32_t x= 0; x < memcached_server_count(memc); x++)
  {
    const memcached_instance_st * instance= memcached_server_instance_by_position(memc, x);

    ids.push_back(get_udp_datagram_request_id((struct udp_datagram_header_st *) ((const memcached_instance_st * )instance)->write_buffer));
  }
}

static test_return_t post_udp_op_check(memcached_st *memc, Expected& expected_req_ids)
{
  (void)memc;
  (void)expected_req_ids;
#if 0
  memcached_server_st *cur_server = memcached_server_list(memc);
  uint16_t *cur_req_ids = get_udp_request_ids(memc);

  for (size_t x= 0; x < memcached_server_count(memc); x++)
  {
    test_true(cur_server[x].cursor_active == 0);
    test_true(cur_req_ids[x] == expected_req_ids[x]);
  }
  free(expected_req_ids);
  free(cur_req_ids);

#endif
  return TEST_SUCCESS;
}

/*
** There is a little bit of a hack here, instead of removing
** the servers, I just set num host to 0 and them add then new udp servers
**/
static test_return_t init_udp(memcached_st *memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP, true));

  return TEST_SUCCESS;
}

static test_return_t init_udp_valgrind(memcached_st *memc)
{
  if (getenv("LOG_COMPILER"))
  {
    return TEST_SKIPPED; 
  }

  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP, true));

  return TEST_SUCCESS;
}

static test_return_t binary_init_udp(memcached_st *memc)
{
  if (getenv("LOG_COMPILER"))
  {
    return TEST_SKIPPED; 
  }

  test_skip(TEST_SUCCESS, pre_binary(memc));

  return init_udp(memc);
}

/* Make sure that I cant add a tcp server to a udp client */
static test_return_t add_tcp_server_udp_client_test(memcached_st *memc)
{
  (void)memc;
#if 0
  memcached_server_st server;
  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);
  memcached_server_clone(&server, &memc->hosts[0]);
  test_true(memcached_server_remove(&(memc->hosts[0])) == MEMCACHED_SUCCESS);
  test_true(memcached_server_add(memc, server.hostname, server.port) == MEMCACHED_INVALID_HOST_PROTOCOL);
#endif
  return TEST_SUCCESS;
}

/* Make sure that I cant add a udp server to a tcp client */
static test_return_t add_udp_server_tcp_client_test(memcached_st *memc)
{
  (void)memc;
#if 0
  memcached_server_st server;
  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);
  memcached_server_clone(&server, &memc->hosts[0]);
  test_true(memcached_server_remove(&(memc->hosts[0])) == MEMCACHED_SUCCESS);

  memcached_st tcp_client;
  memcached_create(&tcp_client);
  test_true(memcached_server_add_udp(&tcp_client, server.hostname, server.port) == MEMCACHED_INVALID_HOST_PROTOCOL);
#endif

  return TEST_SUCCESS;
}

static test_return_t version_TEST(memcached_st *memc)
{
  test_compare(MEMCACHED_NOT_SUPPORTED, memcached_version(memc));
  return TEST_SUCCESS;
}

static test_return_t verbosity_TEST(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_verbosity(memc, 0));
  return TEST_SUCCESS;
}

static test_return_t memcached_get_TEST(memcached_st *memc)
{
  memcached_return_t rc;
  test_null(memcached_get(memc,
                          test_literal_param(__func__),
                          0, 0, &rc));
  test_compare(MEMCACHED_NOT_SUPPORTED, rc);

  return TEST_SUCCESS;
}

static test_return_t memcached_mget_execute_by_key_TEST(memcached_st *memc)
{
  char **keys= NULL;
  size_t *key_length= NULL;
  test_compare(MEMCACHED_NOT_SUPPORTED,
               memcached_mget_execute_by_key(memc,
                                             test_literal_param(__func__), // Group key
                                             keys, key_length, // Actual key
                                             0, // Number of keys
                                             0, // callbacks
                                             0, // context
                                             0)); // Number of callbacks

  return TEST_SUCCESS;
}

static test_return_t memcached_stat_TEST(memcached_st *memc)
{
  memcached_return_t rc;
  test_null(memcached_stat(memc, 0, &rc));
  test_compare(MEMCACHED_NOT_SUPPORTED, rc);

  return TEST_SUCCESS;
}

static test_return_t set_udp_behavior_test(memcached_st *memc)
{
  memcached_quit(memc);

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, memc->distribution));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP, true));
  test_compare(true, memc->flags.use_udp);
  test_compare(false, memc->flags.reply);

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP, false));
  test_compare(false, memc->flags.use_udp);
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NOREPLY, false));
  test_compare(true, memc->flags.reply);

  return TEST_SUCCESS;
}

static test_return_t udp_set_test(memcached_st *memc)
{
  // Assume we are running under valgrind, and bail 
  if (getenv("LOG_COMPILER"))
  {
    return TEST_SUCCESS; 
  }

  const unsigned int num_iters= 1025; //request id rolls over at 1024

  test_true(memc);

  for (size_t x= 0; x < num_iters;x++)
  {
    Expected expected_ids;
    get_udp_request_ids(memc, expected_ids);
    unsigned int server_key= memcached_generate_hash(memc, test_literal_param("foo"));
    test_true(server_key < memcached_server_count(memc));
    const memcached_instance_st * instance= memcached_server_instance_by_position(memc, server_key);
    size_t init_offset= instance->write_buffer_offset;

    test_compare_hint(MEMCACHED_SUCCESS, 
                      memcached_set(memc,
                                    test_literal_param("foo"),
                                    test_literal_param("when we sanitize"),
                                    time_t(0), uint32_t(0)),
                      memcached_last_error_message(memc));

    /*
      NB, the check below assumes that if new write_ptr is less than
      the original write_ptr that we have flushed. For large payloads, this
      maybe an invalid assumption, but for the small payload we have it is OK
    */
    if (instance->write_buffer_offset < init_offset)
    {
      increment_request_id(&expected_ids[server_key]);
    }

    test_compare(TEST_SUCCESS, post_udp_op_check(memc, expected_ids));
  }

  return TEST_SUCCESS;
}

static test_return_t udp_buffered_set_test(memcached_st *memc)
{
  test_true(memc);
  test_compare(MEMCACHED_INVALID_ARGUMENTS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, true));
  return TEST_SUCCESS;
}

static test_return_t udp_set_too_big_test(memcached_st *memc)
{
  test_true(memc);
  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);

  std::vector<char> value;
  value.resize(1024 * 1024 * 10);

  test_compare_hint(MEMCACHED_WRITE_FAILURE,
                    memcached_set(memc,
                                  test_literal_param(__func__), 
                                  &value[0], value.size(),
                                  time_t(0), uint32_t(0)),
                    memcached_last_error_message(memc));
  memcached_quit(memc);

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_delete_test(memcached_st *memc)
{
  test_true(memc);

  //request id rolls over at 1024
  for (size_t x= 0; x < 1025; x++)
  {
    Expected expected_ids;
    get_udp_request_ids(memc, expected_ids);

    unsigned int server_key= memcached_generate_hash(memc, test_literal_param("foo"));
    const memcached_instance_st * instance= memcached_server_instance_by_position(memc, server_key);
    size_t init_offset= instance->write_buffer_offset;

    test_compare(MEMCACHED_SUCCESS,
                 memcached_delete(memc, test_literal_param("foo"), 0));

    if (instance->write_buffer_offset < init_offset)
    {
      increment_request_id(&expected_ids[server_key]);
    }

    test_compare(TEST_SUCCESS, post_udp_op_check(memc, expected_ids));
  }

  return TEST_SUCCESS;
}

static test_return_t udp_buffered_delete_test(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 1);
  return udp_delete_test(memc);
}

static test_return_t udp_verbosity_test(memcached_st *memc)
{
  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);

  for (size_t x= 0; x < memcached_server_count(memc); x++)
  {
    increment_request_id(&expected_ids[x]);
  }

  test_compare(MEMCACHED_SUCCESS, memcached_verbosity(memc, 3));

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_quit_test(memcached_st *memc)
{
  Expected expected_ids;
  memcached_quit(memc);

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_flush_test(memcached_st *memc)
{
  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);

  for (size_t x= 0; x < memcached_server_count(memc); x++)
  {
    increment_request_id(&expected_ids[x]);
  }
  memcached_error_print(memc);
  test_compare_hint(MEMCACHED_SUCCESS, memcached_flush(memc, 0), memcached_last_error_message(memc));

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_incr_test(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc, test_literal_param("incr"), 
                             test_literal_param("1"),
                             (time_t)0, (uint32_t)0));

  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);

  unsigned int server_key= memcached_generate_hash(memc, test_literal_param("incr"));
  increment_request_id(&expected_ids[server_key]);

  uint64_t newvalue;
  test_compare(MEMCACHED_SUCCESS, memcached_increment(memc, test_literal_param("incr"), 1, &newvalue));

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_decr_test(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc, 
                             test_literal_param(__func__),
                             test_literal_param("1"),
                             time_t(0), uint32_t(0)));

  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);

  unsigned int server_key= memcached_generate_hash(memc,
                                                   test_literal_param(__func__));
  increment_request_id(&expected_ids[server_key]);

  uint64_t newvalue;
  test_compare(MEMCACHED_SUCCESS, memcached_decrement(memc,
                                                      test_literal_param(__func__),
                                                      1, &newvalue));

  return post_udp_op_check(memc, expected_ids);
}


static test_return_t udp_stat_test(memcached_st *memc)
{
  memcached_return_t rc;
  char args[]= "";
  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);
  memcached_stat_st *rv= memcached_stat(memc, args, &rc);
  memcached_stat_free(memc, rv);
  test_compare(MEMCACHED_NOT_SUPPORTED, rc);

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_version_test(memcached_st *memc)
{
  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);

  test_compare(MEMCACHED_NOT_SUPPORTED,
               memcached_version(memc));

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_get_test(memcached_st *memc)
{
  memcached_return_t rc;
  size_t vlen;
  Expected expected_ids;
  get_udp_request_ids(memc, expected_ids);
  test_null(memcached_get(memc, test_literal_param("foo"), &vlen, NULL, &rc));
  test_compare(MEMCACHED_NOT_SUPPORTED, rc);

  return post_udp_op_check(memc, expected_ids);
}

static test_return_t udp_mixed_io_test(memcached_st *memc)
{
  test_st mixed_io_ops [] ={
    {"udp_set_test", 0,
      (test_callback_fn*)udp_set_test},
    {"udp_set_too_big_test", 0,
      (test_callback_fn*)udp_set_too_big_test},
    {"udp_delete_test", 0,
      (test_callback_fn*)udp_delete_test},
    {"udp_verbosity_test", 0,
      (test_callback_fn*)udp_verbosity_test},
    {"udp_quit_test", 0,
      (test_callback_fn*)udp_quit_test},
#if 0
    {"udp_flush_test", 0,
      (test_callback_fn*)udp_flush_test},
#endif
    {"udp_incr_test", 0,
      (test_callback_fn*)udp_incr_test},
    {"udp_decr_test", 0,
      (test_callback_fn*)udp_decr_test},
    {"udp_version_test", 0,
      (test_callback_fn*)udp_version_test}
  };

  for (size_t x= 0; x < 500; x++)
  {
    test_st current_op= mixed_io_ops[(random() % 8)];
    test_compare(TEST_SUCCESS, current_op.test_fn(memc));
  }
  return TEST_SUCCESS;
}

test_st compatibility_TESTS[] ={
  {"version", 0, (test_callback_fn*)version_TEST },
  {"version", 0, (test_callback_fn*)verbosity_TEST },
  {"memcached_get()", 0, (test_callback_fn*)memcached_get_TEST },
  {"memcached_mget_execute_by_key()", 0, (test_callback_fn*)memcached_mget_execute_by_key_TEST },
  {"memcached_stat()", 0, (test_callback_fn*)memcached_stat_TEST },
  {0, 0, 0}
};

test_st udp_setup_server_tests[] ={
  {"set_udp_behavior_test", 0, (test_callback_fn*)set_udp_behavior_test},
  {"add_tcp_server_udp_client_test", 0, (test_callback_fn*)add_tcp_server_udp_client_test},
  {"add_udp_server_tcp_client_test", 0, (test_callback_fn*)add_udp_server_tcp_client_test},
  {0, 0, 0}
};

test_st upd_io_tests[] ={
  {"udp_set_test", 0, (test_callback_fn*)udp_set_test},
  {"udp_buffered_set_test", 0, (test_callback_fn*)udp_buffered_set_test},
  {"udp_set_too_big_test", 0, (test_callback_fn*)udp_set_too_big_test},
  {"udp_delete_test", 0, (test_callback_fn*)udp_delete_test},
  {"udp_buffered_delete_test", 0, (test_callback_fn*)udp_buffered_delete_test},
  {"udp_verbosity_test", 0, (test_callback_fn*)udp_verbosity_test},
  {"udp_quit_test", 0, (test_callback_fn*)udp_quit_test},
  {"udp_flush_test", 0, (test_callback_fn*)udp_flush_test},
  {"udp_incr_test", 0, (test_callback_fn*)udp_incr_test},
  {"udp_decr_test", 0, (test_callback_fn*)udp_decr_test},
  {"udp_stat_test", 0, (test_callback_fn*)udp_stat_test},
  {"udp_version_test", 0, (test_callback_fn*)udp_version_test},
  {"udp_get_test", 0, (test_callback_fn*)udp_get_test},
  {"udp_mixed_io_test", 0, (test_callback_fn*)udp_mixed_io_test},
  {0, 0, 0}
};

collection_st collection[] ={
  {"udp_setup", (test_callback_fn*)init_udp, 0, udp_setup_server_tests},
  {"compatibility", (test_callback_fn*)init_udp, 0, compatibility_TESTS},
  {"udp_io", (test_callback_fn*)init_udp_valgrind, 0, upd_io_tests},
  {"udp_binary_io", (test_callback_fn*)binary_init_udp, 0, upd_io_tests},
  {0, 0, 0, 0}
};

#include "tests/libmemcached_world.h"

void get_world(libtest::Framework* world)
{
  world->collections(collection);

  world->create((test_callback_create_fn*)world_create);
  world->destroy((test_callback_destroy_fn*)world_destroy);

  world->set_runner(new LibmemcachedRunner);
}
