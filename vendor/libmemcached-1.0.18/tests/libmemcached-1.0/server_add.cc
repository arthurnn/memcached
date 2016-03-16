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
#include <libtest/yatl.h>

#include <string>

using namespace libtest;

#include <libmemcached-1.0/memcached.h>

#include <tests/server_add.h>

static std::string random_hostname()
{
  libtest::vchar_t hostname;
  libtest::vchar::make(hostname, 23);
  libtest::vchar::append(hostname, ".com");

  return std::string(&hostname[0]);
}

test_return_t memcached_server_add_null_test(memcached_st* memc)
{
  ASSERT_EQ(0, memcached_server_count(memc));

  test_compare(MEMCACHED_SUCCESS, memcached_server_add(memc, NULL, 0));

  return TEST_SUCCESS;
}

test_return_t memcached_server_add_empty_test(memcached_st* memc)
{
  ASSERT_EQ(0, memcached_server_count(memc));

  test_compare(MEMCACHED_SUCCESS, memcached_server_add(memc, "", 0));

  return TEST_SUCCESS;
}

test_return_t memcached_server_many_TEST(memcached_st* memc)
{
  ASSERT_EQ(0, memcached_server_count(memc));

  in_port_t base_port= 5555;
  for (in_port_t x= 0; x < 100; ++x)
  {
    std::string hostname(random_hostname());
    ASSERT_TRUE(hostname.size());
    test_compare(MEMCACHED_SUCCESS, memcached_server_add(memc, hostname.c_str(), base_port +x));
  }

  return TEST_SUCCESS;
}

test_return_t memcached_server_many_weighted_TEST(memcached_st* memc)
{
  SKIP_IF(true);
  ASSERT_EQ(0, memcached_server_count(memc));

  in_port_t base_port= 5555;
  for (in_port_t x= 0; x < 100; ++x)
  {
    std::string hostname(random_hostname());
    ASSERT_TRUE(hostname.size());
    test_compare(MEMCACHED_SUCCESS, memcached_server_add_with_weight(memc, hostname.c_str(), base_port +x, random() % 10));
  }

  return TEST_SUCCESS;
}
