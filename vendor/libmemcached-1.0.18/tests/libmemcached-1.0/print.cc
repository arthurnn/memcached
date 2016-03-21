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

using namespace libtest;


#include <iostream>

#include <libmemcached-1.0/memcached.h>

#include "tests/print.h"

memcached_return_t server_print_callback(const memcached_st*,
                                         const memcached_instance_st * server,
                                         void *context)
{
  if (context)
  {
    std::cerr << memcached_server_name(server) << ":" << memcached_server_port(server) << std::endl;
  }

  return MEMCACHED_SUCCESS;
}

memcached_return_t server_print_version_callback(const memcached_st *,
                                                 const memcached_instance_st * server,
                                                 void *)
{
  std::cerr << "Server: " << memcached_server_name(server) << ":" << memcached_server_port(server) << " " 
    << int(memcached_server_major_version(server)) << "."
    << int(memcached_server_minor_version(server)) << "."
    << int(memcached_server_micro_version(server))
    << std::endl;

  return MEMCACHED_SUCCESS;
}

const char * print_version(memcached_st *memc)
{
  memcached_server_fn callbacks[1];
  callbacks[0]= server_print_version_callback;
  memcached_server_cursor(memc, callbacks, NULL,  1);

  return "print_version()";
}
