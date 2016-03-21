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

#include <tests/virtual_buckets.h>

#include <libmemcached-1.0/memcached.h>

#include <cstring>

struct libtest_string_t {
  const char *c_str;
  size_t size;
};

static inline libtest_string_t libtest_string(const char *arg, size_t arg_size)
{
  libtest_string_t local= { arg, arg_size };
  return local;
}

#define make_libtest_string(X) libtest_string((X), static_cast<size_t>(sizeof(X) - 1))

static libtest_string_t libtest_string_t_null= { 0, 0};

static bool libtest_string_is_null(const libtest_string_t &string)
{
  if (string.c_str == 0 and string.size == 0)
    return true;

  return false;
}

struct expect_t {
  libtest_string_t key;
  uint32_t server_id;
  uint32_t bucket_id;
};

expect_t basic_keys[]= {
  { make_libtest_string("hello"), 0, 0 },
  { make_libtest_string("doctor"), 0, 0 },
  { make_libtest_string("name"), 1, 3 },
  { make_libtest_string("continue"), 1, 3 },
  { make_libtest_string("yesterday"), 0, 0 },
  { make_libtest_string("tomorrow"), 1, 1 },
  { make_libtest_string("another key"), 2, 2 },
  { libtest_string_t_null, 0, 0 }
};

test_return_t virtual_back_map(memcached_st *)
{
  memcached_return_t rc;
  memcached_server_st *server_pool;
  memcached_st *memc;

  memc= memcached_create(NULL);
  test_true(memc);

  uint32_t server_map[] = { 0, 1, 2, 1 };
  rc= memcached_bucket_set(memc, server_map, NULL, 4, 2);
  test_true(rc == MEMCACHED_SUCCESS);

  memcached_server_distribution_t dt;
  dt= memcached_behavior_get_distribution(memc);
  test_true(dt == MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET);

  memcached_behavior_set_key_hash(memc, MEMCACHED_HASH_CRC);
  test_true(rc == MEMCACHED_SUCCESS);

  memcached_hash_t hash_type= memcached_behavior_get_key_hash(memc);
  test_true(hash_type == MEMCACHED_HASH_CRC);

  server_pool = memcached_servers_parse("localhost:11211, localhost1:11210, localhost2:11211");
  test_true(server_pool);
  memcached_server_push(memc, server_pool);

  /* verify that the server list was parsed okay. */
  test_true(memcached_server_count(memc) == 3);
  test_true(strcmp(server_pool[0].hostname, "localhost") == 0);
  test_true(server_pool[0].port == 11211);

  test_true(strcmp(server_pool[1].hostname, "localhost1") == 0);
  test_true(server_pool[1].port == 11210);

  test_true(strcmp(server_pool[2].hostname, "localhost2") == 0);
  test_true(server_pool[2].port == 11211);

  dt= memcached_behavior_get_distribution(memc);
  hash_type= memcached_behavior_get_key_hash(memc);
  test_true(dt == MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET);
  test_true(hash_type == MEMCACHED_HASH_CRC);

  /* verify the standard ketama set. */
  for (expect_t *ptr= basic_keys; not libtest_string_is_null(ptr->key); ptr++)
  {
    uint32_t server_idx = memcached_generate_hash(memc, ptr->key.c_str, ptr->key.size); 

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%.*s:%lu Got/Expected %u == %u", (int)ptr->key.size, ptr->key.c_str, (unsigned long)ptr->key.size, server_idx, ptr->server_id);
    test_compare(server_idx, ptr->server_id);
  }

  memcached_server_list_free(server_pool);
  memcached_free(memc);

  return TEST_SUCCESS;
}
