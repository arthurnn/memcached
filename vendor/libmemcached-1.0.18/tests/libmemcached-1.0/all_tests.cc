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

#include "tests/basic.h"
#include "tests/debug.h"
#include "tests/deprecated.h"
#include "tests/error_conditions.h"
#include "tests/exist.h"
#include "tests/ketama.h"
#include "tests/namespace.h"
#include "tests/libmemcached-1.0/dump.h"
#include "tests/libmemcached-1.0/generate.h"
#include "tests/libmemcached-1.0/haldenbrand.h"
#include "tests/libmemcached-1.0/parser.h"
#include "tests/libmemcached-1.0/stat.h"
#include "tests/touch.h"
#include "tests/callbacks.h"
#include "tests/pool.h"
#include "tests/print.h"
#include "tests/replication.h"
#include "tests/server_add.h"
#include "tests/virtual_buckets.h"

#include "tests/libmemcached-1.0/setup_and_teardowns.h"


#include "tests/libmemcached-1.0/mem_functions.h"
#include "tests/libmemcached-1.0/encoding_key.h"

/* Collections we are running */
#include "tests/libmemcached-1.0/all_tests.h"

#include "tests/libmemcached_world.h"

#include <algorithm>

void get_world(libtest::Framework* world)
{
  if (getenv("LIBMEMCACHED_SERVER_NUMBER"))
  {
    unsigned long int set_count= strtoul(getenv("LIBMEMCACHED_SERVER_NUMBER"), (char **) NULL, 10);
    fatal_assert(set_count >= 1);
    world->servers().set_servers_to_run(set_count);
  }
  else
  {
    // Assume a minimum of 3, and a maximum of 8
    world->servers().set_servers_to_run((libtest::number_of_cpus() > 3) ? 
                                        std::min(libtest::number_of_cpus(), size_t(8)) : 3);
  }

  world->collections(collection);

  world->create((test_callback_create_fn*)world_create);
  world->destroy((test_callback_destroy_fn*)world_destroy);

  world->set_runner(new LibmemcachedRunner);

  world->set_socket();
}
