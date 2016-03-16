/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached
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

#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

test_return_t memcached_NULL_string_TEST(memcached_st*);
test_return_t memcached_zero_string_length_TEST(memcached_st*);

LIBTEST_LOCAL
test_return_t server_test(memcached_st *memc);

LIBTEST_LOCAL
test_return_t servers_bad_test(memcached_st *memc);

LIBTEST_LOCAL
test_return_t behavior_parser_test(memcached_st*);

LIBTEST_LOCAL
test_return_t parser_number_options_test(memcached_st*);

LIBTEST_LOCAL
test_return_t parser_distribution_test(memcached_st*);

LIBTEST_LOCAL
test_return_t parser_hash_test(memcached_st*);

LIBTEST_LOCAL
test_return_t parser_boolean_options_test(memcached_st*);

LIBTEST_LOCAL
test_return_t parser_key_prefix_test(memcached_st*);

LIBTEST_LOCAL
  test_return_t libmemcached_check_configuration_test(memcached_st*);

LIBTEST_LOCAL
  test_return_t memcached_create_with_options_test(memcached_st*);

LIBTEST_LOCAL
  test_return_t memcached_create_with_options_with_filename(memcached_st*);

LIBTEST_LOCAL
  test_return_t libmemcached_check_configuration_with_filename_test(memcached_st*);

LIBTEST_LOCAL
  test_return_t random_statement_build_test(memcached_st*);

LIBTEST_LOCAL
test_return_t test_include_keyword(memcached_st*);

LIBTEST_LOCAL
test_return_t test_end_keyword(memcached_st*);

LIBTEST_LOCAL
test_return_t test_reset_keyword(memcached_st*);

LIBTEST_LOCAL
test_return_t test_error_keyword(memcached_st*);

LIBTEST_LOCAL
test_return_t server_with_weight_test(memcached_st *);

LIBTEST_LOCAL
test_return_t test_hostname_port_weight(memcached_st *);

LIBTEST_LOCAL
test_return_t regression_bug_71231153_connect(memcached_st *);

LIBTEST_LOCAL
test_return_t regression_bug_71231153_poll(memcached_st *);

LIBTEST_LOCAL
test_return_t test_parse_socket(memcached_st *);

LIBTEST_LOCAL
test_return_t test_namespace_keyword(memcached_st*);

#ifdef	__cplusplus
}
#endif
