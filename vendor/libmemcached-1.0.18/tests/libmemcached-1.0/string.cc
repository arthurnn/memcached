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

// We let libmemcached/common.h define config since we are looking at
// library internals.

#include <mem_config.h>

#include <libmemcached-1.0/memcached.h>

#include "libmemcached/string.hpp"
#include "libmemcached/is.h"

#include <libtest/test.hpp>

#include <tests/string.h>

test_return_t string_static_null(void*)
{
  memcached_st *memc= memcached_create(NULL);
  memcached_string_st string;

  memcached_string_st *string_ptr= memcached_string_create(memc, &string, 0);
  test_true(string.options.is_initialized);
  test_true(string_ptr);

  /* The following two better be the same! */
  test_false(memcached_is_allocated(string_ptr));
  test_false(memcached_is_allocated(&string));
  test_true(&string == string_ptr);

  test_true(string.options.is_initialized);
  test_true(memcached_is_initialized(&string));
  memcached_string_free(&string);
  test_false(memcached_is_initialized(&string));

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t string_alloc_null(void*)
{
  memcached_st *memc= memcached_create(NULL);

  memcached_string_st *string= memcached_string_create(memc, NULL, 0);
  test_true(string);
  test_true(memcached_is_allocated(string));
  test_true(memcached_is_initialized(string));
  memcached_string_free(string);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t string_alloc_with_size(void*)
{
  memcached_st *memc= memcached_create(NULL);
  memcached_string_st *string= memcached_string_create(memc, NULL, 1024);
  test_true(string);
  test_true(memcached_is_allocated(string));
  test_true(memcached_is_initialized(string));
  memcached_string_free(string);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t string_alloc_with_size_toobig(void*)
{
  memcached_st *memc= memcached_create(NULL);
  memcached_string_st *string= memcached_string_create(memc, NULL, SIZE_MAX);
  test_zero(string);
  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t string_alloc_append(void*)
{
  memcached_st *memc= memcached_create(NULL);

  char buffer[BUFSIZ];
  memcached_string_st *string;

  /* Ring the bell! */
  memset(buffer, 6, BUFSIZ);

  string= memcached_string_create(memc, NULL, 100);
  test_true(string);
  test_true(memcached_is_allocated(string));
  test_true(memcached_is_initialized(string));

  for (unsigned int x= 0; x < 1024; x++)
  {
    memcached_return_t rc;
    rc= memcached_string_append(string, buffer, BUFSIZ);
    test_true(rc == MEMCACHED_SUCCESS);
  }
  test_true(memcached_is_allocated(string));
  memcached_string_free(string);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t string_alloc_append_toobig(void*)
{
  memcached_st *memc= memcached_create(NULL);

  memcached_return_t rc;
  char buffer[BUFSIZ];
  memcached_string_st *string;

  /* Ring the bell! */
  memset(buffer, 6, sizeof(buffer));

  string= memcached_string_create(memc, NULL, 100);
  test_true(string);
  test_true(memcached_is_allocated(string));
  test_true(memcached_is_initialized(string));

  for (unsigned int x= 0; x < 1024; x++)
  {
    rc= memcached_string_append(string, buffer, BUFSIZ);
    test_true(rc == MEMCACHED_SUCCESS);
  }
  rc= memcached_string_append(string, buffer, SIZE_MAX);
  test_true(rc == MEMCACHED_MEMORY_ALLOCATION_FAILURE);
  test_true(memcached_is_allocated(string));
  memcached_string_free(string);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t string_alloc_append_multiple(void*)
{
  memcached_st *memc= memcached_create(NULL);

  memcached_string_st *error_string= memcached_string_create(memc, NULL, 1024);
  memcached_string_append(error_string, test_literal_param("Error occured while parsing: "));
  memcached_string_append(error_string, test_string_make_from_cstr("jog the strlen() method"));
  memcached_string_append(error_string, test_literal_param(" ("));

  memcached_string_append(error_string, test_string_make_from_cstr(memcached_strerror(NULL, MEMCACHED_SUCCESS)));
  memcached_string_append(error_string, test_literal_param(")"));

  memcached_string_free(error_string);

  memcached_free(memc);

  return TEST_SUCCESS;
}
