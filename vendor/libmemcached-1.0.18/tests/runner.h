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


#pragma once

#include "tests/libmemcached-1.0/generate.h"
#include "tests/memc.hpp"
#include "tests/print.h"

class LibmemcachedRunner : public libtest::Runner {
public:
  test_return_t run(test_callback_fn* func, void *object)
  {
    return _runner_default(libmemcached_test_callback_fn(func), (libmemcached_test_container_st*)object);
  }

  test_return_t flush(void* arg)
  {
    return flush((libmemcached_test_container_st*)arg);
  }

  test_return_t flush(libmemcached_test_container_st *container)
  {
    test::Memc memc(container->parent());
    memcached_flush(&memc, 0);
    memcached_quit(&memc);

    return TEST_SUCCESS;
  }

  test_return_t pre(test_callback_fn* func, void *object)
  {
    return _pre_runner_default(libmemcached_test_callback_fn(func), (libmemcached_test_container_st*)object);
  }

  test_return_t post(test_callback_fn* func, void *object)
  {
    return _post_runner_default(libmemcached_test_callback_fn(func), (libmemcached_test_container_st*)object);
  }

private:
  test_return_t _runner_default(libmemcached_test_callback_fn func, libmemcached_test_container_st *container)
  {
    test_true(container);
    test_true(container->parent());
    test::Memc memc(container->parent());

    test_compare(true, check());

    test_return_t ret= TEST_SUCCESS;
    if (func)
    {
      test_true(container);
      ret= func(&memc);
    }

    return ret;
  }

  test_return_t _pre_runner_default(libmemcached_test_callback_fn func, libmemcached_test_container_st *container)
  {
    container->reset();
    {
      char buffer[BUFSIZ];

      test_compare(MEMCACHED_SUCCESS,
                   libmemcached_check_configuration(container->construct.option_string().c_str(), container->construct.option_string().size(),
                                                    buffer, sizeof(buffer)));

      test_null(container->parent());
      container->parent(memcached(container->construct.option_string().c_str(), container->construct.option_string().size()));
      test_true(container->parent());
#if 0
      test_compare(MEMCACHED_SUCCESS, memcached_version(container->parent()));
#endif

      if (container->construct.sasl())
      {
        if (memcached_failed(memcached_behavior_set(container->parent(), MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1)))
        {
          container->reset();
          return TEST_FAILURE;
        }

        if (memcached_failed(memcached_set_sasl_auth_data(container->parent(), container->construct.username().c_str(), container->construct.password().c_str())))
        {
          container->reset();
          return TEST_FAILURE;
        }
      }
    }

    test_compare(true, check());

    if (func)
    {
      return func(container->parent());
    }

    return TEST_SUCCESS;
  }

  test_return_t _post_runner_default(libmemcached_test_callback_fn func, libmemcached_test_container_st *container)
  {
    test_compare(true, check());
    cleanup_pairs(NULL);

    test_return_t rc= TEST_SUCCESS;
    if (func)
    {
      rc= func(container->parent());
    }
    container->reset();

    return rc;
  }
};

