/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached Client and Server 
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker
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

#include <cassert>

#include "tests/libmemcached_test_container.h"

static void *world_create(libtest::server_startup_st& servers, test_return_t& error)
{
  if (libtest::has_memcached() == false)
  {
    error= TEST_SKIPPED;
    return NULL;
  }

  for (uint32_t x= 0; x < servers.servers_to_run(); x++)
  {
    const char *argv[]= { "memcached", 0 };
    if (servers.start_socket_server("memcached", libtest::get_free_port(), argv) == false)
    {
#if 0
      fatal_message("Could not launch memcached");
#endif
      error= TEST_SKIPPED;
      return NULL;
    }
  }


  libmemcached_test_container_st *global_container= new libmemcached_test_container_st(servers);

  error= TEST_SUCCESS;

  return global_container;
}

static bool world_destroy(void *object)
{
  libmemcached_test_container_st *container= (libmemcached_test_container_st *)object;

#if 0
#if defined(LIBMEMCACHED_WITH_SASL_SUPPORT) && LIBMEMCACHED_WITH_SASL_SUPPORT
  if (LIBMEMCACHED_WITH_SASL_SUPPORT)
  {
    sasl_done();
  }
#endif
#endif

  delete container;

  return TEST_SUCCESS;
}

typedef test_return_t (*libmemcached_test_callback_fn)(memcached_st *);

#include "tests/runner.h"
