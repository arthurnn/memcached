/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
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

#include <mem_config.h>
#include <libtest/test.hpp>

#include <libmemcached-1.0/memcached.h>

#include "libmemcached/server_instance.h"
#include "libmemcached/continuum.hpp"
#include "libmemcached/instance.hpp"

#include <tests/ketama.h>
#include <tests/ketama_test_cases.h>

test_return_t ketama_compatibility_libmemcached(memcached_st *)
{
  memcached_st *memc= memcached_create(NULL);
  test_true(memc);

  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, 1));

  test_compare(uint64_t(1), memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED));

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set_distribution(memc, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA));
  test_compare(MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA, memcached_behavior_get_distribution(memc));

  memcached_server_st *server_pool= memcached_servers_parse("10.0.1.1:11211 600,10.0.1.2:11211 300,10.0.1.3:11211 200,10.0.1.4:11211 350,10.0.1.5:11211 1000,10.0.1.6:11211 800,10.0.1.7:11211 950,10.0.1.8:11211 100");
  memcached_server_push(memc, server_pool);

  /* verify that the server list was parsed okay. */
  test_compare(8U, memcached_server_count(memc));
  test_strcmp(server_pool[0].hostname, "10.0.1.1");
  test_compare(in_port_t(11211), server_pool[0].port);
  test_compare(600U, server_pool[0].weight);
  test_strcmp(server_pool[2].hostname, "10.0.1.3");
  test_compare(in_port_t(11211), server_pool[2].port);
  test_compare(200U, server_pool[2].weight);
  test_strcmp(server_pool[7].hostname, "10.0.1.8");
  test_compare(in_port_t(11211), server_pool[7].port);
  test_compare(100U, server_pool[7].weight);

  /* VDEAAAAA hashes to fffcd1b5, after the last continuum point, and lets
   * us test the boundary wraparound.
   */
  test_true(memcached_generate_hash(memc, (char *)"VDEAAAAA", 8) == memc->ketama.continuum[0].index);

  /* verify the standard ketama set. */
  for (uint32_t x= 0; x < 99; x++)
  {
    uint32_t server_idx = memcached_generate_hash(memc, ketama_test_cases[x].key, strlen(ketama_test_cases[x].key));
    const memcached_instance_st * instance=
      memcached_server_instance_by_position(memc, server_idx);
    const char *hostname = memcached_server_name(instance);

    test_strcmp(hostname, ketama_test_cases[x].server);
  }

  memcached_server_list_free(server_pool);
  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t user_supplied_bug18(memcached_st *trash)
{
  memcached_return_t rc;
  uint64_t value;
  int x;
  memcached_st *memc;

  (void)trash;

  memc= memcached_create(NULL);
  test_true(memc);

  rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, 1);
  test_compare(MEMCACHED_SUCCESS, rc);

  value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED);
  test_true(value == 1);

  rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH, MEMCACHED_HASH_MD5);
  test_compare(MEMCACHED_SUCCESS, rc);

  value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH);
  test_true(value == MEMCACHED_HASH_MD5);

  memcached_server_st *server_pool= memcached_servers_parse("10.0.1.1:11211 600,10.0.1.2:11211 300,10.0.1.3:11211 200,10.0.1.4:11211 350,10.0.1.5:11211 1000,10.0.1.6:11211 800,10.0.1.7:11211 950,10.0.1.8:11211 100");
  memcached_server_push(memc, server_pool);

  /* verify that the server list was parsed okay. */
  test_true(memcached_server_count(memc) == 8);
  test_strcmp(server_pool[0].hostname, "10.0.1.1");
  test_true(server_pool[0].port == 11211);
  test_true(server_pool[0].weight == 600);
  test_strcmp(server_pool[2].hostname, "10.0.1.3");
  test_true(server_pool[2].port == 11211);
  test_true(server_pool[2].weight == 200);
  test_strcmp(server_pool[7].hostname, "10.0.1.8");
  test_true(server_pool[7].port == 11211);
  test_true(server_pool[7].weight == 100);

  /* VDEAAAAA hashes to fffcd1b5, after the last continuum point, and lets
   * us test the boundary wraparound.
   */
  test_true(memcached_generate_hash(memc, (char *)"VDEAAAAA", 8) == memc->ketama.continuum[0].index);

  /* verify the standard ketama set. */
  for (x= 0; x < 99; x++)
  {
    uint32_t server_idx = memcached_generate_hash(memc, ketama_test_cases[x].key, strlen(ketama_test_cases[x].key));

    const memcached_instance_st * instance=
      memcached_server_instance_by_position(memc, server_idx);

    const char *hostname = memcached_server_name(instance);
    test_strcmp(hostname, ketama_test_cases[x].server);
  }

  memcached_server_list_free(server_pool);
  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t auto_eject_hosts(memcached_st *trash)
{
  (void) trash;

  memcached_return_t rc;
  memcached_st *memc= memcached_create(NULL);
  test_true(memc);

  rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, 1);
  test_compare(MEMCACHED_SUCCESS, rc);

  uint64_t value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED);
  test_true(value == 1);

  rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH, MEMCACHED_HASH_MD5);
  test_compare(MEMCACHED_SUCCESS, rc);

  value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH);
  test_true(value == MEMCACHED_HASH_MD5);

    /* server should be removed when in delay */
  rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS, 1);
  test_compare(MEMCACHED_SUCCESS, rc);

  value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS);
  test_true(value == 1);

  memcached_server_st *server_pool;
  server_pool = memcached_servers_parse("10.0.1.1:11211 600,10.0.1.2:11211 300,10.0.1.3:11211 200,10.0.1.4:11211 350,10.0.1.5:11211 1000,10.0.1.6:11211 800,10.0.1.7:11211 950,10.0.1.8:11211 100");
  memcached_server_push(memc, server_pool);

  /* verify that the server list was parsed okay. */
  test_true(memcached_server_count(memc) == 8);
  test_strcmp(server_pool[0].hostname, "10.0.1.1");
  test_true(server_pool[0].port == 11211);
  test_true(server_pool[0].weight == 600);
  test_strcmp(server_pool[2].hostname, "10.0.1.3");
  test_true(server_pool[2].port == 11211);
  test_true(server_pool[2].weight == 200);
  test_strcmp(server_pool[7].hostname, "10.0.1.8");
  test_true(server_pool[7].port == 11211);
  test_true(server_pool[7].weight == 100);

  const memcached_instance_st * instance= memcached_server_instance_by_position(memc, 2);
  memcached_instance_next_retry(instance, time(NULL) +15);
  memc->ketama.next_distribution_rebuild= time(NULL) - 1;

  /*
    This would not work if there were only two hosts.
  */
  for (ptrdiff_t x= 0; x < 99; x++)
  {
    memcached_autoeject(memc);
    uint32_t server_idx= memcached_generate_hash(memc, ketama_test_cases[x].key, strlen(ketama_test_cases[x].key));
    test_true(server_idx != 2);
  }

  /* and re-added when it's back. */
  time_t absolute_time= time(NULL) -1;
  memcached_instance_next_retry(instance, absolute_time);
  memc->ketama.next_distribution_rebuild= absolute_time;
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION,
                         memc->distribution);
  for (ptrdiff_t x= 0; x < 99; x++)
  {
    uint32_t server_idx = memcached_generate_hash(memc, ketama_test_cases[x].key, strlen(ketama_test_cases[x].key));
    // We re-use instance from above.
    instance=
      memcached_server_instance_by_position(memc, server_idx);
    const char *hostname = memcached_server_name(instance);
    test_strcmp(hostname, ketama_test_cases[x].server);
  }

  memcached_server_list_free(server_pool);
  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t ketama_compatibility_spymemcached(memcached_st *)
{
  memcached_st *memc= memcached_create(NULL);
  test_true(memc);

  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, 1));

  test_compare(UINT64_C(1), memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED));

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set_distribution(memc, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY));
  test_compare(MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY, memcached_behavior_get_distribution(memc));

  memcached_server_st *server_pool= memcached_servers_parse("10.0.1.1:11211 600,10.0.1.2:11211 300,10.0.1.3:11211 200,10.0.1.4:11211 350,10.0.1.5:11211 1000,10.0.1.6:11211 800,10.0.1.7:11211 950,10.0.1.8:11211 100");
  test_true(server_pool);
  memcached_server_push(memc, server_pool);

  /* verify that the server list was parsed okay. */
  test_compare(8U, memcached_server_count(memc));
  test_strcmp(server_pool[0].hostname, "10.0.1.1");
  test_compare(in_port_t(11211), server_pool[0].port);
  test_compare(600U, server_pool[0].weight);
  test_strcmp(server_pool[2].hostname, "10.0.1.3");
  test_compare(in_port_t(11211), server_pool[2].port);
  test_compare(200U, server_pool[2].weight);
  test_strcmp(server_pool[7].hostname, "10.0.1.8");
  test_compare(in_port_t(11211), server_pool[7].port);
  test_compare(100U, server_pool[7].weight);

  /* VDEAAAAA hashes to fffcd1b5, after the last continuum point, and lets
   * us test the boundary wraparound.
   */
  test_true(memcached_generate_hash(memc, (char *)"VDEAAAAA", 8) == memc->ketama.continuum[0].index);

  /* verify the standard ketama set. */
  for (uint32_t x= 0; x < 99; x++)
  {
    uint32_t server_idx= memcached_generate_hash(memc, ketama_test_cases_spy[x].key, strlen(ketama_test_cases_spy[x].key));

    const memcached_instance_st * instance=
      memcached_server_instance_by_position(memc, server_idx);

    const char *hostname= memcached_server_name(instance);

    test_strcmp(hostname, ketama_test_cases_spy[x].server);
  }

  memcached_server_list_free(server_pool);
  memcached_free(memc);

  return TEST_SUCCESS;
}
