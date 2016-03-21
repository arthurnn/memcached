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

#include <libmemcached-1.0/memcached.h>

#include <tests/namespace.h>

test_return_t memcached_increment_namespace(memcached_st *memc)
{
  uint64_t new_number;

  test_compare(MEMCACHED_SUCCESS, 
               memcached_set(memc, 
                             test_literal_param("number"),
                             test_literal_param("0"),
                             (time_t)0, (uint32_t)0));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_increment(memc,
                                   test_literal_param("number"),
                                   1, &new_number));
  test_compare(uint64_t(1), new_number);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_increment(memc,
                                   test_literal_param("number"),
                                   1, &new_number));
  test_compare(uint64_t(2), new_number);

  memcached_st *clone= memcached_clone(NULL, memc);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_callback_set(clone, MEMCACHED_CALLBACK_NAMESPACE, "all_your_bases"));

  test_compare(MEMCACHED_NOTFOUND, 
               memcached_increment(clone,
                                   test_literal_param("number"),
                                   1, &new_number));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_add(clone, 
                             test_literal_param("number"),
                             test_literal_param("10"),
                             (time_t)0, (uint32_t)0));

  char *value= memcached_get(clone, 
                             test_literal_param("number"),
                             0, 0, 0);
  test_true(value);
  test_compare(2UL, strlen(value));
  test_strcmp("10", value);
  free(value);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_increment(clone,
                                   test_literal_param("number"),
                                   1, &new_number));
  test_compare(uint64_t(11), new_number);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_increment(memc,
                                   test_literal_param("number"),
                                   1, &new_number));
  test_compare(uint64_t(3), new_number);

  memcached_free(clone);

  return TEST_SUCCESS;
}

