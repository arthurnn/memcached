/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker All rights reserved.
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
#include <tests/deprecated.h>

test_return_t server_list_null_test(memcached_st *ptr)
{
  memcached_server_st *server_list;
  memcached_return_t rc;
  (void)ptr;

  server_list= memcached_server_list_append_with_weight(NULL, NULL, 0, 0, NULL);
  test_true(server_list);
  memcached_server_list_free(server_list);

  server_list= memcached_server_list_append_with_weight(NULL, "localhost", 0, 0, NULL);
  test_true(server_list);
  memcached_server_list_free(server_list);

  server_list= memcached_server_list_append_with_weight(NULL, NULL, 0, 0, &rc);
  test_true(server_list);
  memcached_server_list_free(server_list);

  return TEST_SUCCESS;
}

// Look for memory leak
test_return_t regression_bug_728286(memcached_st *)
{
  memcached_server_st *servers= memcached_servers_parse("1.2.3.4:99");
  fatal_assert(servers);
  memcached_server_free(servers);

  return TEST_SUCCESS;
}

