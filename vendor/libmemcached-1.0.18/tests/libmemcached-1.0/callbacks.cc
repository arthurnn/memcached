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

#include <tests/callbacks.h>

using namespace libtest;

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

static memcached_return_t delete_trigger(memcached_st *,
                                         const char *key,
                                         size_t key_length)
{
  fatal_assert(key);
  fatal_assert(key_length);

  return MEMCACHED_SUCCESS;
}


test_return_t test_MEMCACHED_CALLBACK_DELETE_TRIGGER_and_MEMCACHED_BEHAVIOR_NOREPLY(memcached_st *)
{
  memcached_st *memc= memcached(test_literal_param("--NOREPLY"));
  test_true(memc);

  memcached_trigger_delete_key_fn callback;

  callback= (memcached_trigger_delete_key_fn)delete_trigger;

  test_compare(MEMCACHED_INVALID_ARGUMENTS, 
               memcached_callback_set(memc, MEMCACHED_CALLBACK_DELETE_TRIGGER, *(void**)&callback));

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t test_MEMCACHED_CALLBACK_DELETE_TRIGGER(memcached_st *memc)
{
  memcached_trigger_delete_key_fn callback;

  callback= (memcached_trigger_delete_key_fn)delete_trigger;

  test_compare(MEMCACHED_SUCCESS, 
               memcached_callback_set(memc, MEMCACHED_CALLBACK_DELETE_TRIGGER, *(void**)&callback));

  return TEST_SUCCESS;
}
