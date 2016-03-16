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

#include <libmemcached-1.0/memcached.h>
#include <libmemcachedutil-1.0/util.h>

#include "tests/libmemcached-1.0/encoding_key.h"

using namespace libtest;

test_return_t memcached_set_encoding_key_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_set_get_TEST(memcached_st* memc)
{
  memcached_st *memc_no_crypt= memcached_clone(NULL, memc);
  test_true(memc_no_crypt);
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_SUCCESS, memcached_set(memc,
                                                test_literal_param(__func__), // Key
                                                test_literal_param(__func__), // Value
                                                time_t(0),
                                                uint32_t(0)));

  {
    memcached_return_t rc;
    size_t value_length;
    char *value;
    test_true((value= memcached_get(memc,
                                    test_literal_param(__func__), // Key
                                    &value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(test_literal_param_size(__func__), value_length);
    test_memcmp(__func__, value, value_length);

    size_t raw_value_length;
    char *raw_value;
    test_true((raw_value= memcached_get(memc_no_crypt,
                                        test_literal_param(__func__), // Key
                                        &raw_value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_ne_compare(value_length, raw_value_length);
    test_ne_compare(0, memcmp(value, raw_value, raw_value_length));

    free(value);
    free(raw_value);
  }

  memcached_free(memc_no_crypt);

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_add_get_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_SUCCESS, memcached_add(memc,
                                                     test_literal_param(__func__), // Key
                                                     test_literal_param(__func__), // Value
                                                     time_t(0),
                                                     uint32_t(0)));

  {
    memcached_return_t rc;
    size_t value_length;
    char *value;
    test_true((value= memcached_get(memc,
                                    test_literal_param(__func__), // Key
                                    &value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(test_literal_param_size(__func__), value_length);
    test_memcmp(__func__, value, value_length);
    free(value);
  }

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_replace_get_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  // First we add the key
  {
    test_compare(MEMCACHED_SUCCESS, memcached_add(memc,
                                                  test_literal_param(__func__), // Key
                                                  test_literal_param(__func__), // Value
                                                  time_t(0),
                                                  uint32_t(0)));

    memcached_return_t rc;
    size_t value_length;
    char *value;
    test_true((value= memcached_get(memc,
                                    test_literal_param(__func__), // Key
                                    &value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(test_literal_param_size(__func__), value_length);
    test_memcmp(__func__, value, value_length);
    free(value);
  }
 
  // Then we replace the key
  {
    libtest::vchar_t new_value;
    vchar::make(new_value);

    test_compare(MEMCACHED_SUCCESS, memcached_replace(memc,
                                                      test_literal_param(__func__), // Key
                                                      vchar_param(new_value), // Value
                                                      time_t(0),
                                                      uint32_t(0)));

    memcached_return_t rc;
    size_t value_length;
    char *value;
    test_true((value= memcached_get(memc,
                                    test_literal_param(__func__), // Key
                                    &value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(new_value.size(), value_length);
    test_compare(0, vchar::compare(new_value, value, value_length));
    free(value);
  }

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_increment_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_NOT_SUPPORTED, memcached_increment(memc,
                                                            test_literal_param(__func__), // Key
                                                            uint32_t(0),
                                                            NULL));

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_decrement_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_NOT_SUPPORTED, memcached_decrement(memc,
                                                            test_literal_param(__func__), // Key
                                                            uint32_t(0),
                                                            NULL));

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_increment_with_initial_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_NOT_SUPPORTED, memcached_increment_with_initial(memc,
                                                                         test_literal_param(__func__), // Key
                                                                         uint32_t(0),
                                                                         uint32_t(0),
                                                                         time_t(0),
                                                                         NULL));

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_decrement_with_initial_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_NOT_SUPPORTED, memcached_decrement_with_initial(memc,
                                                                         test_literal_param(__func__), // Key
                                                                         uint32_t(0),
                                                                         uint32_t(0),
                                                                         time_t(0),
                                                                         NULL));

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_append_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_NOT_SUPPORTED, memcached_append(memc,
                                                         test_literal_param(__func__), // Key
                                                         test_literal_param(__func__), // Value
                                                         time_t(0),
                                                         uint32_t(0)));

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_prepend_TEST(memcached_st* memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  test_compare(MEMCACHED_NOT_SUPPORTED, memcached_prepend(memc,
                                                         test_literal_param(__func__), // Key
                                                         test_literal_param(__func__), // Value
                                                         time_t(0),
                                                         uint32_t(0)));

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_set_get_clone_TEST(memcached_st* memc)
{
  memcached_st *memc_no_crypt= memcached_clone(NULL, memc);
  test_true(memc_no_crypt);

  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));
  
  memcached_st *memc_crypt= memcached_clone(NULL, memc);
  test_true(memc_crypt);

  test_compare(MEMCACHED_SUCCESS, memcached_set(memc,
                                                     test_literal_param(__func__), // Key
                                                     test_literal_param(__func__), // Value
                                                     time_t(0),
                                                     uint32_t(0)));

  {
    memcached_return_t rc;
    size_t value_length;
    char *value;
    test_true((value= memcached_get(memc,
                                    test_literal_param(__func__), // Key
                                    &value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(test_literal_param_size(__func__), value_length);
    test_memcmp(__func__, value, value_length);

    /*
      Check to make sure that the raw value is not the original.
    */
    size_t raw_value_length;
    char *raw_value;
    test_true((raw_value= memcached_get(memc_no_crypt,
                                        test_literal_param(__func__), // Key
                                        &raw_value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_ne_compare(test_literal_param_size(__func__), raw_value_length);
    test_ne_compare(0, memcmp(__func__, raw_value, raw_value_length));

    /*
      Now we will use our clone, and make sure the encrypted values are the same.
    */
    size_t second_value_length;
    char *second_value;
    test_true((second_value= memcached_get(memc_crypt,
                                           test_literal_param(__func__), // Key
                                           &second_value_length, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(value_length, second_value_length);
    test_compare(0, memcmp(value, second_value, second_value_length));
    test_compare(test_literal_param_size(__func__), second_value_length);
    test_compare(value_length, second_value_length);
    test_memcmp(__func__, second_value, second_value_length);
    test_memcmp(value, second_value, second_value_length);

    free(value);
    free(raw_value);
    free(second_value);
  }

  memcached_free(memc_no_crypt);
  memcached_free(memc_crypt);

  return TEST_SUCCESS;
}

test_return_t memcached_set_encoding_key_set_grow_key_TEST(memcached_st* memc)
{
  memcached_st *memc_no_crypt= memcached_clone(NULL, memc);
  test_true(memc_no_crypt);
  test_compare(MEMCACHED_SUCCESS, memcached_set_encoding_key(memc, test_literal_param(__func__)));

  size_t payload_size[] = { 100, 1000, 10000, 1000000, 1000000, 0 };
  libtest::vchar_t payload;
  for (size_t *ptr= payload_size; *ptr; ptr++)
  {
    payload.reserve(*ptr);
    for (size_t x= payload.size(); x < *ptr; x++)
    { 
      payload.push_back(rand());
    }

    {
      memcached_return_t rc= memcached_set(memc,
                                           test_literal_param(__func__), // Key
                                           &payload[0], payload.size(), // Value
                                           time_t(0),
                                           uint32_t(0));

      // If we run out of space on the server, we just end the test early.
      if (rc == MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE)
      {
        break;
      }
      test_compare(MEMCACHED_SUCCESS, rc);
    }

    {
      memcached_return_t rc;
      size_t value_length;
      char *value;
      test_true((value= memcached_get(memc,
                                      test_literal_param(__func__), // Key
                                      &value_length, NULL, &rc)));
      test_compare(MEMCACHED_SUCCESS, rc);
      test_compare(payload.size(), value_length);
      test_memcmp(&payload[0], value, value_length);

      size_t raw_value_length;
      char *raw_value;
      test_true((raw_value= memcached_get(memc_no_crypt,
                                          test_literal_param(__func__), // Key
                                          &raw_value_length, NULL, &rc)));
      test_compare(MEMCACHED_SUCCESS, rc);
      test_ne_compare(payload.size(), raw_value_length);
      test_ne_compare(0, memcmp(&payload[0], raw_value, raw_value_length));

      free(value);
      free(raw_value);
    }
  }

  memcached_free(memc_no_crypt);

  return TEST_SUCCESS;
}
