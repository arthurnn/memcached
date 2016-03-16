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

#include "tests/libmemcached-1.0/memcached_get.h"


/* Clean the server before beginning testing */
test_st tests[] ={
  {"util_version", true, (test_callback_fn*)util_version_test },
  {"flush", false, (test_callback_fn*)flush_test },
  {"init", false, (test_callback_fn*)init_test },
  {"allocation", false, (test_callback_fn*)allocation_test },
  {"server_list_null_test", false, (test_callback_fn*)server_list_null_test},
  {"server_unsort", false, (test_callback_fn*)server_unsort_test},
  {"server_sort", false, (test_callback_fn*)server_sort_test},
  {"server_sort2", false, (test_callback_fn*)server_sort2_test},
  {"memcached_server_remove", false, (test_callback_fn*)memcached_server_remove_test},
  {"clone_test", false, (test_callback_fn*)clone_test },
  {"connection_test", false, (test_callback_fn*)connection_test},
  {"callback_test", false, (test_callback_fn*)callback_test},
  {"userdata_test", false, (test_callback_fn*)userdata_test},
  {"memcached_set()", false, (test_callback_fn*)set_test },
  {"memcached_set() 2", false, (test_callback_fn*)set_test2 },
  {"memcached_set() 3", false, (test_callback_fn*)set_test3 },
  {"memcached_add(SUCCESS)", true, (test_callback_fn*)memcached_add_SUCCESS_TEST },
  {"add", true, (test_callback_fn*)add_test },
  {"memcached_fetch_result(MEMCACHED_NOTFOUND)", true, (test_callback_fn*)memcached_fetch_result_NOT_FOUND },
  {"replace", true, (test_callback_fn*)replace_test },
  {"delete", true, (test_callback_fn*)delete_test },
  {"memcached_get()", true, (test_callback_fn*)get_test },
  {"get2", false, (test_callback_fn*)get_test2 },
  {"get3", false, (test_callback_fn*)get_test3 },
  {"get4", false, (test_callback_fn*)get_test4 },
  {"partial mget", false, (test_callback_fn*)get_test5 },
  {"stats_servername", false, (test_callback_fn*)stats_servername_test },
  {"increment", false, (test_callback_fn*)increment_test },
  {"memcached_increment_with_initial(0)", true, (test_callback_fn*)increment_with_initial_test },
  {"memcached_increment_with_initial(999)", true, (test_callback_fn*)increment_with_initial_999_test },
  {"decrement", false, (test_callback_fn*)decrement_test },
  {"memcached_decrement_with_initial(3)", true, (test_callback_fn*)decrement_with_initial_test },
  {"memcached_decrement_with_initial(999)", true, (test_callback_fn*)decrement_with_initial_999_test },
  {"increment_by_key", false, (test_callback_fn*)increment_by_key_test },
  {"increment_with_initial_by_key", true, (test_callback_fn*)increment_with_initial_by_key_test },
  {"decrement_by_key", false, (test_callback_fn*)decrement_by_key_test },
  {"decrement_with_initial_by_key", true, (test_callback_fn*)decrement_with_initial_by_key_test },
  {"binary_increment_with_prefix", true, (test_callback_fn*)binary_increment_with_prefix_test },
  {"quit", false, (test_callback_fn*)quit_test },
  {"mget", true, (test_callback_fn*)mget_test },
  {"mget_result", true, (test_callback_fn*)mget_result_test },
  {"mget_result_alloc", true, (test_callback_fn*)mget_result_alloc_test },
  {"mget_result_function", true, (test_callback_fn*)mget_result_function },
  {"mget_execute", true, (test_callback_fn*)mget_execute },
  {"mget_end", false, (test_callback_fn*)mget_end },
  {"get_stats", false, (test_callback_fn*)get_stats },
  {"add_host_test", false, (test_callback_fn*)add_host_test },
  {"add_host_test_1", false, (test_callback_fn*)add_host_test1 },
  {"get_stats_keys", false, (test_callback_fn*)get_stats_keys },
  {"version_string_test", true, (test_callback_fn*)version_string_test},
  {"memcached_mget() mixed memcached_get()", true, (test_callback_fn*)memcached_mget_mixed_memcached_get_TEST},
  {"bad_key", true, (test_callback_fn*)bad_key_test },
  {"memcached_server_cursor", true, (test_callback_fn*)memcached_server_cursor_test },
  {"read_through", true, (test_callback_fn*)read_through },
  {"delete_through", true, (test_callback_fn*)test_MEMCACHED_CALLBACK_DELETE_TRIGGER },
  {"noreply", true, (test_callback_fn*)noreply_test},
  {"analyzer", true, (test_callback_fn*)analyzer_test},
  {"memcached_pool_st", true, (test_callback_fn*)connection_pool_test },
  {"memcached_pool_st #2", true, (test_callback_fn*)connection_pool2_test },
#if 0
  {"memcached_pool_st #3", true, (test_callback_fn*)connection_pool3_test },
#endif
  {"memcached_pool_test", true, (test_callback_fn*)memcached_pool_test },
  {"test_get_last_disconnect", true, (test_callback_fn*)test_get_last_disconnect},
  {"verbosity", true, (test_callback_fn*)test_verbosity},
  {"memcached_stat_execute", true, (test_callback_fn*)memcached_stat_execute_test},
  {"memcached_exist(MEMCACHED_NOTFOUND)", true, (test_callback_fn*)memcached_exist_NOTFOUND },
  {"memcached_exist(MEMCACHED_SUCCESS)", true, (test_callback_fn*)memcached_exist_SUCCESS },
  {"memcached_exist_by_key(MEMCACHED_NOTFOUND)", true, (test_callback_fn*)memcached_exist_by_key_NOTFOUND },
  {"memcached_exist_by_key(MEMCACHED_SUCCESS)", true, (test_callback_fn*)memcached_exist_by_key_SUCCESS },
  {"memcached_touch", 0, (test_callback_fn*)test_memcached_touch},
  {"memcached_touch_with_prefix", 0, (test_callback_fn*)test_memcached_touch_by_key},
#if 0
  {"memcached_dump() no data", true, (test_callback_fn*)memcached_dump_TEST },
#endif
  {"memcached_dump() with data", true, (test_callback_fn*)memcached_dump_TEST2 },
  {0, 0, 0}
};

test_st touch_tests[] ={
  {"memcached_touch", 0, (test_callback_fn*)test_memcached_touch},
  {"memcached_touch_with_prefix", 0, (test_callback_fn*)test_memcached_touch_by_key},
  {0, 0, 0}
};

test_st kill_TESTS[] ={
  {"kill(HUP)", 0, (test_callback_fn*)kill_HUP_TEST},
  {0, 0, 0}
};

test_st memcached_stat_tests[] ={
  {"memcached_stat() INVALID ARG", 0, (test_callback_fn*)memcached_stat_TEST},
  {"memcached_stat()", 0, (test_callback_fn*)memcached_stat_TEST2},
  {0, 0, 0}
};

test_st behavior_tests[] ={
  {"libmemcached_string_behavior()", false, (test_callback_fn*)libmemcached_string_behavior_test},
  {"libmemcached_string_distribution()", false, (test_callback_fn*)libmemcached_string_distribution_test},
  {"behavior_test", false, (test_callback_fn*)behavior_test},
  {"MEMCACHED_BEHAVIOR_CORK", false, (test_callback_fn*)MEMCACHED_BEHAVIOR_CORK_test},
  {"MEMCACHED_BEHAVIOR_TCP_KEEPALIVE", false, (test_callback_fn*)MEMCACHED_BEHAVIOR_TCP_KEEPALIVE_test},
  {"MEMCACHED_BEHAVIOR_TCP_KEEPIDLE", false, (test_callback_fn*)MEMCACHED_BEHAVIOR_TCP_KEEPIDLE_test},
  {"MEMCACHED_BEHAVIOR_POLL_TIMEOUT", false, (test_callback_fn*)MEMCACHED_BEHAVIOR_POLL_TIMEOUT_test},
  {"MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH_TEST", true, (test_callback_fn*)MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH_TEST },
  {"MEMCACHED_CALLBACK_DELETE_TRIGGER_and_MEMCACHED_BEHAVIOR_NOREPLY", false, (test_callback_fn*)test_MEMCACHED_CALLBACK_DELETE_TRIGGER_and_MEMCACHED_BEHAVIOR_NOREPLY},
  {0, 0, 0}
};

test_st libmemcachedutil_tests[] ={
  {"libmemcached_util_ping()", true, (test_callback_fn*)libmemcached_util_ping_TEST },
  {"libmemcached_util_getpid()", true, (test_callback_fn*)getpid_test },
  {"libmemcached_util_getpid(MEMCACHED_CONNECTION_FAILURE)", true, (test_callback_fn*)getpid_connection_failure_test },
  {0, 0, 0}
};

test_st basic_tests[] ={
  {"init", true, (test_callback_fn*)basic_init_test},
  {"clone", true, (test_callback_fn*)basic_clone_test},
  {"reset", true, (test_callback_fn*)basic_reset_stack_test},
  {"reset heap", true, (test_callback_fn*)basic_reset_heap_test},
  {"reset stack clone", true, (test_callback_fn*)basic_reset_stack_clone_test},
  {"reset heap clone", true, (test_callback_fn*)basic_reset_heap_clone_test},
  {"memcached_return_t", false, (test_callback_fn*)memcached_return_t_TEST },
  {"c++ memcached_st == memcached_return_t", false, (test_callback_fn*)comparison_operator_memcached_st_and__memcached_return_t_TEST },
  {0, 0, 0}
};

test_st regression_binary_vs_block[] ={
  {"block add", true, (test_callback_fn*)block_add_regression},
  {"binary add", true, (test_callback_fn*)binary_add_regression},
  {0, 0, 0}
};

test_st async_tests[] ={
  {"add", true, (test_callback_fn*)add_wrapper },
  {0, 0, 0}
};

test_st memcached_server_get_last_disconnect_tests[] ={
  {"memcached_server_get_last_disconnect()", false, (test_callback_fn*)test_multiple_get_last_disconnect},
  {0, 0, (test_callback_fn*)0}
};


test_st result_tests[] ={
  {"result static", false, (test_callback_fn*)result_static},
  {"result alloc", false, (test_callback_fn*)result_alloc},
  {0, 0, (test_callback_fn*)0}
};

test_st version_1_2_3[] ={
  {"append", false, (test_callback_fn*)append_test },
  {"prepend", false, (test_callback_fn*)prepend_test },
  {"cas", false, (test_callback_fn*)cas_test },
  {"cas2", false, (test_callback_fn*)cas2_test },
  {"append_binary", false, (test_callback_fn*)append_binary_test },
  {0, 0, (test_callback_fn*)0}
};

test_st haldenbrand_TESTS[] ={
  {"memcached_set", false, (test_callback_fn*)haldenbrand_TEST1 },
  {"memcached_get()", false, (test_callback_fn*)haldenbrand_TEST2 },
  {"memcached_mget()", false, (test_callback_fn*)haldenbrand_TEST3 },
  {0, 0, (test_callback_fn*)0}
};

test_st user_tests[] ={
  {"user_supplied_bug4", true, (test_callback_fn*)user_supplied_bug4 },
  {"user_supplied_bug5", true, (test_callback_fn*)user_supplied_bug5 },
  {"user_supplied_bug6", true, (test_callback_fn*)user_supplied_bug6 },
  {"user_supplied_bug7", true, (test_callback_fn*)user_supplied_bug7 },
  {"user_supplied_bug8", true, (test_callback_fn*)user_supplied_bug8 },
  {"user_supplied_bug9", true, (test_callback_fn*)user_supplied_bug9 },
  {"user_supplied_bug10", true, (test_callback_fn*)user_supplied_bug10 },
  {"user_supplied_bug11", true, (test_callback_fn*)user_supplied_bug11 },
  {"user_supplied_bug12", true, (test_callback_fn*)user_supplied_bug12 },
  {"user_supplied_bug13", true, (test_callback_fn*)user_supplied_bug13 },
  {"user_supplied_bug14", true, (test_callback_fn*)user_supplied_bug14 },
  {"user_supplied_bug15", true, (test_callback_fn*)user_supplied_bug15 },
  {"user_supplied_bug16", true, (test_callback_fn*)user_supplied_bug16 },
#if !defined(__sun) && !defined(__OpenBSD__)
  /*
   ** It seems to be something weird with the character sets..
   ** value_fetch is unable to parse the value line (iscntrl "fails"), so I
   ** guess I need to find out how this is supposed to work.. Perhaps I need
   ** to run the test in a specific locale (I tried zh_CN.UTF-8 without success,
   ** so just disable the code for now...).
 */
  {"user_supplied_bug17", true, (test_callback_fn*)user_supplied_bug17 },
#endif
  {"user_supplied_bug18", true, (test_callback_fn*)user_supplied_bug18 },
  {"user_supplied_bug19", true, (test_callback_fn*)user_supplied_bug19 },
  {"user_supplied_bug20", true, (test_callback_fn*)user_supplied_bug20 },
  {"user_supplied_bug21", true, (test_callback_fn*)user_supplied_bug21 },
  {"wrong_failure_counter_test", true, (test_callback_fn*)wrong_failure_counter_test},
  {"wrong_failure_counter_two_test", true, (test_callback_fn*)wrong_failure_counter_two_test},
  {0, 0, (test_callback_fn*)0}
};

test_st replication_tests[]= {
  {"validate replication setup", true, (test_callback_fn*)check_replication_sanity_TEST },
  {"set", true, (test_callback_fn*)replication_set_test },
  {"get", false, (test_callback_fn*)replication_get_test },
  {"mget", false, (test_callback_fn*)replication_mget_test },
  {"delete", true, (test_callback_fn*)replication_delete_test },
  {"rand_mget", false, (test_callback_fn*)replication_randomize_mget_test },
  {"miss", false, (test_callback_fn*)replication_miss_test },
  {"fail", false, (test_callback_fn*)replication_randomize_mget_fail_test },
  {0, 0, (test_callback_fn*)0}
};

/*
 * The following test suite is used to verify that we don't introduce
 * regression bugs. If you want more information about the bug / test,
 * you should look in the bug report at
 *   http://bugs.launchpad.net/libmemcached
 */
test_st regression_tests[]= {
  {"lp:434484", true, (test_callback_fn*)regression_bug_434484 },
  {"lp:434843", true, (test_callback_fn*)regression_bug_434843 },
  {"lp:434843-buffered", true, (test_callback_fn*)regression_bug_434843_buffered },
  {"lp:421108", true, (test_callback_fn*)regression_bug_421108 },
  {"lp:442914", true, (test_callback_fn*)regression_bug_442914 },
  {"lp:447342", true, (test_callback_fn*)regression_bug_447342 },
  {"lp:463297", true, (test_callback_fn*)regression_bug_463297 },
  {"lp:490486", true, (test_callback_fn*)regression_bug_490486 },
  {"lp:583031", true, (test_callback_fn*)regression_bug_583031 },
  {"lp:?", true, (test_callback_fn*)regression_bug_ },
  {"lp:728286", true, (test_callback_fn*)regression_bug_728286 },
  {"lp:581030", true, (test_callback_fn*)regression_bug_581030 },
  {"lp:71231153 connect()", true, (test_callback_fn*)regression_bug_71231153_connect },
  {"lp:71231153 poll()", true, (test_callback_fn*)regression_bug_71231153_poll },
  {"lp:655423", true, (test_callback_fn*)regression_bug_655423 },
  {"lp:490520", true, (test_callback_fn*)regression_bug_490520 },
  {"lp:854604", true, (test_callback_fn*)regression_bug_854604 },
  {"lp:996813", true, (test_callback_fn*)regression_996813_TEST },
  {"lp:994772", true, (test_callback_fn*)regression_994772_TEST },
  {"lp:1009493", true, (test_callback_fn*)regression_1009493_TEST },
  {"lp:1021819", true, (test_callback_fn*)regression_1021819_TEST },
  {"lp:1048945", true, (test_callback_fn*)regression_1048945_TEST },
  {"lp:1067242", true, (test_callback_fn*)regression_1067242_TEST },
  {"lp:1251482", true, (test_callback_fn*)regression_bug_1251482 },
  {0, false, (test_callback_fn*)0}
};

test_st ketama_compatibility[]= {
  {"libmemcached", true, (test_callback_fn*)ketama_compatibility_libmemcached },
  {"spymemcached", true, (test_callback_fn*)ketama_compatibility_spymemcached },
  {0, 0, (test_callback_fn*)0}
};

test_st generate_tests[] ={
  {"generate_data", true, (test_callback_fn*)generate_data },
  {"get_read", false, (test_callback_fn*)get_read },
  {"delete_generate", false, (test_callback_fn*)delete_generate },
  {"cleanup", true, (test_callback_fn*)cleanup_pairs },
  {0, 0, (test_callback_fn*)0}
};
  // New start
test_st generate_mget_TESTS[] ={
  {"generate_data", true, (test_callback_fn*)generate_data },
  {"mget_read", false, (test_callback_fn*)mget_read },
  {"mget_read_result", false, (test_callback_fn*)mget_read_result },
  {"memcached_fetch_result() use internal result", false, (test_callback_fn*)mget_read_internal_result },
  {"memcached_fetch_result() partial read", false, (test_callback_fn*)mget_read_partial_result },
  {"mget_read_function", false, (test_callback_fn*)mget_read_function },
  {"cleanup", true, (test_callback_fn*)cleanup_pairs },
  {0, 0, (test_callback_fn*)0}
};

test_st generate_large_TESTS[] ={
  {"generate_large_pairs", true, (test_callback_fn*)generate_large_pairs },
  {"cleanup", true, (test_callback_fn*)cleanup_pairs },
  {0, 0, (test_callback_fn*)0}
};

test_st consistent_tests[] ={
  {"generate_data", true, (test_callback_fn*)generate_data },
  {"get_read", 0, (test_callback_fn*)get_read_count },
  {"cleanup", true, (test_callback_fn*)cleanup_pairs },
  {0, 0, (test_callback_fn*)0}
};

test_st consistent_weighted_tests[] ={
  {"generate_data", true, (test_callback_fn*)generate_data_with_stats },
  {"get_read", false, (test_callback_fn*)get_read_count },
  {"cleanup", true, (test_callback_fn*)cleanup_pairs },
  {0, 0, (test_callback_fn*)0}
};

test_st hsieh_availability[] ={
  {"hsieh_avaibility_test", false, (test_callback_fn*)hsieh_avaibility_test},
  {0, 0, (test_callback_fn*)0}
};

test_st murmur_availability[] ={
  {"murmur_avaibility_test", false, (test_callback_fn*)murmur_avaibility_test},
  {0, 0, (test_callback_fn*)0}
};

#if 0
test_st hash_sanity[] ={
  {"hash sanity", 0, (test_callback_fn*)hash_sanity_test},
  {0, 0, (test_callback_fn*)0}
};
#endif

test_st ketama_auto_eject_hosts[] ={
  {"basic ketama test", true, (test_callback_fn*)ketama_TEST },
  {"auto_eject_hosts", true, (test_callback_fn*)auto_eject_hosts },
  {"output_ketama_weighted_keys", true, (test_callback_fn*)output_ketama_weighted_keys },
  {0, 0, (test_callback_fn*)0}
};

test_st hash_tests[] ={
  {"one_at_a_time_run", false, (test_callback_fn*)one_at_a_time_run },
  {"md5", false, (test_callback_fn*)md5_run },
  {"crc", false, (test_callback_fn*)crc_run },
  {"fnv1_64", false, (test_callback_fn*)fnv1_64_run },
  {"fnv1a_64", false, (test_callback_fn*)fnv1a_64_run },
  {"fnv1_32", false, (test_callback_fn*)fnv1_32_run },
  {"fnv1a_32", false, (test_callback_fn*)fnv1a_32_run },
  {"hsieh", false, (test_callback_fn*)hsieh_run },
  {"murmur", false, (test_callback_fn*)murmur_run },
  {"murmur3", false, (test_callback_fn*)murmur3_TEST },
  {"jenkis", false, (test_callback_fn*)jenkins_run },
  {"memcached_get_hashkit", false, (test_callback_fn*)memcached_get_hashkit_test },
  {0, 0, (test_callback_fn*)0}
};

test_st error_conditions[] ={
  {"memcached_get(MEMCACHED_ERRNO)", false, (test_callback_fn*)memcached_get_MEMCACHED_ERRNO },
  {"memcached_get(MEMCACHED_NOTFOUND)", false, (test_callback_fn*)memcached_get_MEMCACHED_NOTFOUND },
  {"memcached_get_by_key(MEMCACHED_ERRNO)", false, (test_callback_fn*)memcached_get_by_key_MEMCACHED_ERRNO },
  {"memcached_get_by_key(MEMCACHED_NOTFOUND)", false, (test_callback_fn*)memcached_get_by_key_MEMCACHED_NOTFOUND },
  {"memcached_get_by_key(MEMCACHED_NOTFOUND)", false, (test_callback_fn*)memcached_get_by_key_MEMCACHED_NOTFOUND },
  {"memcached_increment(MEMCACHED_NO_SERVERS)", false, (test_callback_fn*)memcached_increment_MEMCACHED_NO_SERVERS },
  {0, 0, (test_callback_fn*)0}
};

test_st parser_tests[] ={
  {"behavior", false, (test_callback_fn*)behavior_parser_test },
  {"boolean_options", false, (test_callback_fn*)parser_boolean_options_test },
  {"configure_file", false, (test_callback_fn*)memcached_create_with_options_with_filename },
  {"distribtions", false, (test_callback_fn*)parser_distribution_test },
  {"hash", false, (test_callback_fn*)parser_hash_test },
  {"libmemcached_check_configuration", false, (test_callback_fn*)libmemcached_check_configuration_test },
  {"libmemcached_check_configuration_with_filename", false, (test_callback_fn*)libmemcached_check_configuration_with_filename_test },
  {"number_options", false, (test_callback_fn*)parser_number_options_test },
  {"randomly generated options", false, (test_callback_fn*)random_statement_build_test },
  {"namespace", false, (test_callback_fn*)parser_key_prefix_test },
  {"server", false, (test_callback_fn*)server_test },
  {"bad server strings", false, (test_callback_fn*)servers_bad_test },
  {"server with weights", false, (test_callback_fn*)server_with_weight_test },
  {"parsing servername, port, and weight", false, (test_callback_fn*)test_hostname_port_weight },
  {"--socket=", false, (test_callback_fn*)test_parse_socket },
  {"--namespace=", false, (test_callback_fn*)test_namespace_keyword },
  {0, 0, (test_callback_fn*)0}
};

test_st virtual_bucket_tests[] ={
  {"basic", false, (test_callback_fn*)virtual_back_map },
  {0, 0, (test_callback_fn*)0}
};

test_st memcached_server_add_TESTS[] ={
  {"memcached_server_add(\"\")", false, (test_callback_fn*)memcached_server_add_empty_test },
  {"memcached_server_add(NULL)", false, (test_callback_fn*)memcached_server_add_null_test },
  {"memcached_server_add(many)", false, (test_callback_fn*)memcached_server_many_TEST },
  {"memcached_server_add(many weighted)", false, (test_callback_fn*)memcached_server_many_weighted_TEST },
  {0, 0, (test_callback_fn*)0}
};

test_st pool_TESTS[] ={
  {"lp:962815", true, (test_callback_fn*)regression_bug_962815 },
  {0, 0, (test_callback_fn*)0}
};

test_st memcached_set_encoding_key_TESTS[] ={
  {"memcached_set_encoding_key()", true, (test_callback_fn*)memcached_set_encoding_key_TEST },
  {"memcached_set_encoding_key() +set() + get()", true, (test_callback_fn*)memcached_set_encoding_key_set_get_TEST },
  {"memcached_set_encoding_key() +add() + get()", true, (test_callback_fn*)memcached_set_encoding_key_add_get_TEST },
  {"memcached_set_encoding_key() +replace() + get()", true, (test_callback_fn*)memcached_set_encoding_key_replace_get_TEST },
  {"memcached_set_encoding_key() +prepend()", true, (test_callback_fn*)memcached_set_encoding_key_prepend_TEST },
  {"memcached_set_encoding_key() +append()", true, (test_callback_fn*)memcached_set_encoding_key_append_TEST },
  {"memcached_set_encoding_key() +increment()", true, (test_callback_fn*)memcached_set_encoding_key_increment_TEST },
  {"memcached_set_encoding_key() +decrement()", true, (test_callback_fn*)memcached_set_encoding_key_increment_TEST },
  {"memcached_set_encoding_key() +increment_with_initial()", true, (test_callback_fn*)memcached_set_encoding_key_increment_with_initial_TEST },
  {"memcached_set_encoding_key() +decrement_with_initial()", true, (test_callback_fn*)memcached_set_encoding_key_decrement_with_initial_TEST },
  {"memcached_set_encoding_key() +set() +get() +cloen()", true, (test_callback_fn*)memcached_set_encoding_key_set_get_clone_TEST },
  {"memcached_set_encoding_key() +set() +get() increase value size", true, (test_callback_fn*)memcached_set_encoding_key_set_grow_key_TEST },
  {0, 0, (test_callback_fn*)0}
};

test_st namespace_tests[] ={
  {"basic tests", true, (test_callback_fn*)selection_of_namespace_tests },
  {"increment", true, (test_callback_fn*)memcached_increment_namespace },
  {0, 0, (test_callback_fn*)0}
};

collection_st collection[] ={
#if 0
  {"hash_sanity", 0, 0, hash_sanity},
#endif
  {"libmemcachedutil", 0, 0, libmemcachedutil_tests},
  {"basic", 0, 0, basic_tests},
  {"hsieh_availability", 0, 0, hsieh_availability},
  {"murmur_availability", 0, 0, murmur_availability},
  {"memcached_server_add", (test_callback_fn*)memcached_servers_reset_SETUP, 0, memcached_server_add_TESTS},
  {"memcached_server_add(MEMCACHED_DISTRIBUTION_CONSISTENT)", (test_callback_fn*)memcached_servers_reset_MEMCACHED_DISTRIBUTION_CONSISTENT_SETUP, 0, memcached_server_add_TESTS},
  {"memcached_server_add(MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED)", (test_callback_fn*)memcached_servers_reset_MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED_SETUP, 0, memcached_server_add_TESTS},
  {"block", 0, 0, tests},
  {"binary", (test_callback_fn*)pre_binary, 0, tests},
  {"nonblock", (test_callback_fn*)pre_nonblock, 0, tests},
  {"nodelay", (test_callback_fn*)pre_nodelay, 0, tests},
  {"settimer", (test_callback_fn*)pre_settimer, 0, tests},
  {"md5", (test_callback_fn*)pre_md5, 0, tests},
  {"crc", (test_callback_fn*)pre_crc, 0, tests},
  {"hsieh", (test_callback_fn*)pre_hsieh, 0, tests},
  {"jenkins", (test_callback_fn*)pre_jenkins, 0, tests},
  {"fnv1_64", (test_callback_fn*)pre_hash_fnv1_64, 0, tests},
  {"fnv1a_64", (test_callback_fn*)pre_hash_fnv1a_64, 0, tests},
  {"fnv1_32", (test_callback_fn*)pre_hash_fnv1_32, 0, tests},
  {"fnv1a_32", (test_callback_fn*)pre_hash_fnv1a_32, 0, tests},
  {"ketama", (test_callback_fn*)pre_behavior_ketama, 0, tests},
  {"ketama_auto_eject_hosts", (test_callback_fn*)pre_behavior_ketama, 0, ketama_auto_eject_hosts},
  {"unix_socket", (test_callback_fn*)pre_unix_socket, 0, tests},
  {"unix_socket_nodelay", (test_callback_fn*)pre_nodelay, 0, tests},
  {"gets", (test_callback_fn*)enable_cas, 0, tests},
  {"consistent_crc", (test_callback_fn*)enable_consistent_crc, 0, tests},
  {"consistent_hsieh", (test_callback_fn*)enable_consistent_hsieh, 0, tests},
#ifdef MEMCACHED_ENABLE_DEPRECATED
  {"deprecated_memory_allocators", (test_callback_fn*)deprecated_set_memory_alloc, 0, tests},
#endif
  {"memory_allocators", (test_callback_fn*)set_memory_alloc, 0, tests},
  {"namespace", (test_callback_fn*)set_namespace, 0, tests},
  {"namespace(BINARY)", (test_callback_fn*)set_namespace_and_binary, 0, tests},
  {"specific namespace", 0, 0, namespace_tests},
  {"specific namespace(BINARY)", (test_callback_fn*)pre_binary, 0, namespace_tests},
  {"version_1_2_3", (test_callback_fn*)check_for_1_2_3, 0, version_1_2_3},
  {"result", 0, 0, result_tests},
  {"async", (test_callback_fn*)pre_nonblock, 0, async_tests},
  {"async(BINARY)", (test_callback_fn*)pre_nonblock_binary, 0, async_tests},
  {"Cal Haldenbrand's tests", 0, 0, haldenbrand_TESTS},
  {"user written tests", 0, 0, user_tests},
  {"generate", 0, 0, generate_tests},
  {"generate MEMCACHED_BEHAVIOR_BUFFER_REQUESTS", (test_callback_fn*)pre_buffer, 0, generate_tests},
  {"mget generate MEMCACHED_BEHAVIOR_BUFFER_REQUESTS", (test_callback_fn*)pre_buffer, 0, generate_mget_TESTS},
  {"generate large", 0, 0, generate_large_TESTS},
  {"generate_hsieh", (test_callback_fn*)pre_hsieh, 0, generate_tests},
  {"generate_ketama", (test_callback_fn*)pre_behavior_ketama, 0, generate_tests},
  {"generate_hsieh_consistent", (test_callback_fn*)enable_consistent_hsieh, 0, generate_tests},
  {"generate_md5", (test_callback_fn*)pre_md5, 0, generate_tests},
  {"generate_murmur", (test_callback_fn*)pre_murmur, 0, generate_tests},
  {"generate_jenkins", (test_callback_fn*)pre_jenkins, 0, generate_tests},
  {"generate_nonblock", (test_callback_fn*)pre_nonblock, 0, generate_tests},
  {"mget generate_nonblock", (test_callback_fn*)pre_nonblock, 0, generate_mget_TESTS},
  {"consistent_not", 0, 0, consistent_tests},
  {"consistent_ketama", (test_callback_fn*)pre_behavior_ketama, 0, consistent_tests},
  {"consistent_ketama_weighted", (test_callback_fn*)pre_behavior_ketama_weighted, 0, consistent_weighted_tests},
  {"ketama_compat", 0, 0, ketama_compatibility},
  {"test_hashes", 0, 0, hash_tests},
  {"replication", (test_callback_fn*)pre_replication, 0, replication_tests},
  {"replication_noblock", (test_callback_fn*)pre_replication_noblock, 0, replication_tests},
  {"regression", 0, 0, regression_tests},
  {"behaviors", 0, 0, behavior_tests},
  {"regression_binary_vs_block", (test_callback_fn*)key_setup, (test_callback_fn*)key_teardown, regression_binary_vs_block},
  {"error_conditions", 0, 0, error_conditions},
  {"parser", 0, 0, parser_tests},
  {"virtual buckets", 0, 0, virtual_bucket_tests},
  {"memcached_server_get_last_disconnect", 0, 0, memcached_server_get_last_disconnect_tests},
  {"touch", 0, 0, touch_tests},
  {"touch", (test_callback_fn*)pre_binary, 0, touch_tests},
  {"memcached_stat()", 0, 0, memcached_stat_tests},
  {"memcached_pool_create()", 0, 0, pool_TESTS},
  {"memcached_set_encoding_key()", 0, 0, memcached_set_encoding_key_TESTS},
  {"kill()", 0, 0, kill_TESTS},
  {0, 0, 0, 0}
};
