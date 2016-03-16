/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached client and server library.
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
#include <climits>

using namespace libtest;

#include <libmemcached-1.0/memcached.h>
#include <tests/debug.h>
#include <tests/print.h>

#include "libmemcached/instance.hpp"

/* Dump each server's keys */
static memcached_return_t print_keys_callback(const memcached_st *,
                                              const char *key,
                                              size_t key_length,
                                              void *)
{

  Out << "\t" << key << " (" << key_length << ")";
  

  return MEMCACHED_SUCCESS;
}

static memcached_return_t server_wrapper_for_dump_callback(const memcached_st *,
                                                           const memcached_instance_st * server,
                                                           void *)
{
  memcached_st *memc= memcached_create(NULL);

  if (strcmp(memcached_server_type(server), "SOCKET") == 0)
  {
    if (memcached_failed(memcached_server_add_unix_socket(memc, memcached_server_name(server))))
    {
      return MEMCACHED_FAILURE;
    }
  }
  else
  {
    if (memcached_failed(memcached_server_add(memc, memcached_server_name(server), memcached_server_port(server))))
    {
      return MEMCACHED_FAILURE;
    }
  }

  memcached_dump_fn callbacks[1];

  callbacks[0]= &print_keys_callback;

  Out << memcached_server_name(server) << ":" << memcached_server_port(server);

  if (memcached_failed(memcached_dump(memc, callbacks, NULL, 1)))
  {
    return MEMCACHED_FAILURE;
  }

  memcached_free(memc);

  return MEMCACHED_SUCCESS;
}


test_return_t confirm_keys_exist(memcached_st *memc, const char * const *keys, const size_t number_of_keys, bool key_matches_value, bool require_all)
{
  for (size_t x= 0; x < number_of_keys; ++x)
  {
    memcached_return_t rc;
    size_t value_length;
    char *value= memcached_get(memc,
                               test_string_make_from_cstr(keys[x]), // Keys
                               &value_length,
                               0, &rc);
    if (require_all)
    {
      test_true(value);
      if (key_matches_value)
      {
        test_strcmp(keys[x], value);
      }
    }
    else if (memcached_success(rc))
    {
      test_warn(value, "get() did not return a value");
      if (value and key_matches_value)
      {
        test_strcmp(keys[x], value);
      }
    }

    if (value)
    {
      free(value);
    }
  }

  return TEST_SUCCESS;
}

test_return_t confirm_keys_dont_exist(memcached_st *memc, const char * const *keys, const size_t number_of_keys)
{
  for (size_t x= 0; x < number_of_keys; ++x)
  {
    memcached_return_t rc;
    size_t value_length;
    char *value= memcached_get(memc,
                               test_string_make_from_cstr(keys[x]), // Keys
                               &value_length,
                               0, &rc);
    test_false(value);
    test_compare(MEMCACHED_NOTFOUND, rc);
  }

  return TEST_SUCCESS;
}


test_return_t print_keys_by_server(memcached_st *memc)
{
  memcached_server_fn callback[]= { server_wrapper_for_dump_callback };
  test_compare(MEMCACHED_SUCCESS, memcached_server_cursor(memc, callback, NULL, test_array_length(callback)));

  return TEST_SUCCESS;
}

static memcached_return_t callback_dump_counter(const memcached_st *ptr,
                                                const char *key,
                                                size_t key_length,
                                                void *context)
{
  (void)ptr; (void)key; (void)key_length;
  size_t *counter= (size_t *)context;

  *counter= *counter + 1;

  return MEMCACHED_SUCCESS;
}

size_t confirm_key_count(memcached_st *memc)
{
  memcached_st *clone= memcached_clone(NULL, memc);
  if (memcached_failed(memcached_behavior_set(clone, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, false)))
  {
    memcached_free(clone);
    return ULONG_MAX;
  }

  memcached_dump_fn callbacks[1];

  callbacks[0]= &callback_dump_counter;

  size_t count= 0;
  if (memcached_failed(memcached_dump(clone, callbacks, (void *)&count, 1)))
  {
    memcached_free(clone);
    return ULONG_MAX;
  }

  memcached_free(clone);
  return count;
}

void print_servers(memcached_st *memc)
{
  memcached_server_fn callbacks[1];
  callbacks[0]= server_print_callback;
  memcached_server_cursor(memc, callbacks, NULL,  1);
}
