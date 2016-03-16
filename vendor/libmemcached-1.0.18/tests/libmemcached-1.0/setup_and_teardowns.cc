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

#include "tests/print.h"
#include "tests/libmemcached-1.0/setup_and_teardowns.h"

#include <sys/stat.h>

using namespace libtest;

memcached_return_t return_value_based_on_buffering(memcached_st *memc)
{
  if (memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS))
  {
    return MEMCACHED_BUFFERED;
  }

  return MEMCACHED_SUCCESS;
}

/**
  @note This should be testing to see if the server really supports the binary protocol.
*/
test_return_t pre_binary(memcached_st *memc)
{
  test_true(memcached_server_count(memc) > 0);
  test_skip(true, libmemcached_util_version_check(memc, 1, 4, 4));
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, true));

  return TEST_SUCCESS;
}

test_return_t pre_buffer(memcached_st *memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, true));

  return TEST_SUCCESS;
}

test_return_t pre_unix_socket(memcached_st *memc)
{
  struct stat buf;

  memcached_servers_reset(memc);
  const char *socket_file= libtest::default_socket();
  test_skip(true, bool(socket_file));

  test_skip(0, stat(socket_file, &buf));

  test_compare(MEMCACHED_SUCCESS,
               memcached_server_add_unix_socket_with_weight(memc, socket_file, 0));

  return TEST_SUCCESS;
}

test_return_t pre_nodelay(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 0);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, 0);

  return TEST_SUCCESS;
}

test_return_t pre_settimer(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SND_TIMEOUT, 1000);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RCV_TIMEOUT, 1000);

  return TEST_SUCCESS;
}

test_return_t pre_murmur(memcached_st *memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_MURMUR));
  return TEST_SUCCESS;
}

test_return_t pre_jenkins(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_JENKINS);

  return TEST_SKIPPED;
}


test_return_t pre_md5(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_MD5);

  return TEST_SUCCESS;
}

test_return_t pre_crc(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_CRC);

  return TEST_SUCCESS;
}

test_return_t pre_hsieh(memcached_st *memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_HSIEH));
  return TEST_SUCCESS;
}

test_return_t pre_hash_fnv1_64(memcached_st *memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_MURMUR));

  return TEST_SUCCESS;
}

test_return_t pre_hash_fnv1a_64(memcached_st *memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_FNV1A_64));

  return TEST_SUCCESS;
}

test_return_t pre_hash_fnv1_32(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_FNV1_32);

  return TEST_SUCCESS;
}

test_return_t pre_hash_fnv1a_32(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_FNV1A_32);

  return TEST_SUCCESS;
}

test_return_t memcached_servers_reset_SETUP(memcached_st *memc)
{
  memcached_servers_reset(memc);
  test_compare(0U, memcached_server_count(memc));
  return TEST_SUCCESS;
}

test_return_t memcached_servers_reset_MEMCACHED_DISTRIBUTION_CONSISTENT_SETUP(memcached_st *memc)
{
  test_compare(TEST_SUCCESS, memcached_servers_reset_SETUP(memc));

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set_distribution(memc, MEMCACHED_DISTRIBUTION_CONSISTENT));
  test_compare(memcached_behavior_get_distribution(memc), MEMCACHED_DISTRIBUTION_CONSISTENT);

  return TEST_SUCCESS;
}

test_return_t memcached_servers_reset_MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED_SETUP(memcached_st *memc)
{
  test_compare(TEST_SUCCESS, memcached_servers_reset_SETUP(memc));
  ASSERT_EQ(0U, memcached_server_count(0));

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set_distribution(memc, MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED));
  test_compare(memcached_behavior_get_distribution(memc), MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED);

  return TEST_SUCCESS;
}

test_return_t pre_behavior_ketama(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA, 1));

  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA), uint64_t(1));

  return TEST_SUCCESS;
}

test_return_t pre_behavior_ketama_weighted(memcached_st *memc)
{
  test_compare(memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, true), MEMCACHED_SUCCESS);

  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED), uint64_t(1));

  test_compare(memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH, MEMCACHED_HASH_MD5), MEMCACHED_SUCCESS);

  test_compare(memcached_hash_t(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH)), MEMCACHED_HASH_MD5);

  return TEST_SUCCESS;
}

test_return_t pre_replication(memcached_st *memc)
{
  test_skip(TEST_SUCCESS, pre_binary(memc));

  /*
   * Make sure that we store the item on all servers
   * (master + replicas == number of servers)
 */
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS, memcached_server_count(memc) - 1));
  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS), uint64_t(memcached_server_count(memc) - 1));

  return TEST_SUCCESS;
}


test_return_t pre_replication_noblock(memcached_st *memc)
{
  test_skip(TEST_SUCCESS, pre_replication(memc));

  return pre_nonblock(memc);
}

test_return_t pre_nonblock(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 0);

  return TEST_SUCCESS;
}

test_return_t pre_nonblock_binary(memcached_st *memc)
{
  memcached_st *memc_clone= memcached_clone(NULL, memc);
  test_true(memc_clone);

  // The memcached_version needs to be done on a clone, because the server
  // will not toggle protocol on an connection.
  memcached_version(memc_clone);

  memcached_return_t rc= MEMCACHED_FAILURE;
  if (libmemcached_util_version_check(memc_clone, 1, 4, 4))
  {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 0);
    test_compare(MEMCACHED_SUCCESS,
                 memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1));
    test_compare(uint64_t(1), memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));
  }
  else
  {
    memcached_free(memc_clone);
    return TEST_SKIPPED;
  }

  memcached_free(memc_clone);

  return rc == MEMCACHED_SUCCESS ? TEST_SUCCESS : TEST_SKIPPED;
}
