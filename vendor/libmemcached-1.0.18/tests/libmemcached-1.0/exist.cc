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

#include <tests/exist.h>

using namespace libtest;

test_return_t memcached_exist_NOTFOUND(memcached_st *memc)
{
  test_compare(MEMCACHED_NOTFOUND, memcached_exist(memc, test_literal_param("frog")));
  return TEST_SUCCESS;
}

test_return_t memcached_exist_SUCCESS(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set(memc, test_literal_param("frog"), 0, 0, 0, 0));
  test_compare(MEMCACHED_SUCCESS, memcached_exist(memc, test_literal_param("frog")));
  test_compare(MEMCACHED_SUCCESS, memcached_delete(memc, test_literal_param("frog"), 0));
  test_compare(MEMCACHED_NOTFOUND, memcached_exist(memc, test_literal_param("frog")));

  return TEST_SUCCESS;
}

test_return_t memcached_exist_by_key_NOTFOUND(memcached_st *memc)
{
  test_compare(MEMCACHED_NOTFOUND, memcached_exist_by_key(memc, test_literal_param("master"), test_literal_param("frog")));
  return TEST_SUCCESS;
}

test_return_t memcached_exist_by_key_SUCCESS(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_by_key(memc, test_literal_param("master"), test_literal_param("frog"), 0, 0, 0, 0));
  test_compare(MEMCACHED_SUCCESS, memcached_exist_by_key(memc, test_literal_param("master"), test_literal_param("frog")));
  test_compare(MEMCACHED_SUCCESS, memcached_delete_by_key(memc, test_literal_param("master"), test_literal_param("frog"), 0));
  test_compare(MEMCACHED_NOTFOUND, memcached_exist_by_key(memc, test_literal_param("master"), test_literal_param("frog")));

  return TEST_SUCCESS;
}
