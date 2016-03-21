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

test_return_t MEMCACHED_BEHAVIOR_CORK_test(memcached_st *memc);
test_return_t MEMCACHED_BEHAVIOR_POLL_TIMEOUT_test(memcached_st *memc);
test_return_t MEMCACHED_BEHAVIOR_TCP_KEEPALIVE_test(memcached_st *memc);
test_return_t MEMCACHED_BEHAVIOR_TCP_KEEPIDLE_test(memcached_st *memc);
test_return_t _user_supplied_bug21(memcached_st* memc, size_t key_count);
test_return_t add_host_test(memcached_st *memc);
test_return_t add_host_test1(memcached_st *memc);
test_return_t memcached_add_SUCCESS_TEST(memcached_st*);
test_return_t add_test(memcached_st *memc);
test_return_t add_wrapper(memcached_st *memc);
test_return_t allocation_test(memcached_st *not_used);
test_return_t analyzer_test(memcached_st *memc);
test_return_t append_binary_test(memcached_st *memc);
test_return_t append_test(memcached_st *memc);
test_return_t bad_key_test(memcached_st *memc);
test_return_t behavior_test(memcached_st *memc);
test_return_t binary_add_regression(memcached_st *memc);
test_return_t binary_increment_with_prefix_test(memcached_st *memc);
test_return_t block_add_regression(memcached_st *memc);
test_return_t callback_test(memcached_st *memc);
test_return_t cas2_test(memcached_st *memc);
test_return_t cas_test(memcached_st *memc);
test_return_t check_for_1_2_3(memcached_st *memc);
test_return_t clone_test(memcached_st *memc);
test_return_t connection_test(memcached_st *memc);
test_return_t crc_run (memcached_st *);
test_return_t decrement_by_key_test(memcached_st *memc);
test_return_t decrement_test(memcached_st *memc);
test_return_t decrement_with_initial_by_key_test(memcached_st *memc);
test_return_t decrement_with_initial_test(memcached_st *memc);
test_return_t decrement_with_initial_999_test(memcached_st *memc);
test_return_t delete_test(memcached_st *memc);
test_return_t deprecated_set_memory_alloc(memcached_st *memc);
test_return_t enable_cas(memcached_st *memc);
test_return_t enable_consistent_crc(memcached_st *memc);
test_return_t enable_consistent_hsieh(memcached_st *memc);
test_return_t flush_test(memcached_st *memc);
test_return_t fnv1_32_run (memcached_st *);
test_return_t fnv1_64_run (memcached_st *);
test_return_t fnv1a_32_run (memcached_st *);
test_return_t fnv1a_64_run (memcached_st *);
test_return_t get_stats(memcached_st *memc);
test_return_t get_stats_keys(memcached_st *memc);
test_return_t getpid_connection_failure_test(memcached_st *memc);
test_return_t getpid_test(memcached_st *memc);
test_return_t hash_sanity_test (memcached_st *memc);
test_return_t hsieh_avaibility_test (memcached_st *memc);
test_return_t hsieh_run (memcached_st *);
test_return_t increment_by_key_test(memcached_st *memc);
test_return_t increment_test(memcached_st *memc);
test_return_t increment_with_initial_by_key_test(memcached_st *memc);
test_return_t increment_with_initial_test(memcached_st *memc);
test_return_t increment_with_initial_999_test(memcached_st *memc);
test_return_t init_test(memcached_st *not_used);
test_return_t jenkins_run (memcached_st *);
test_return_t key_setup(memcached_st *memc);
test_return_t key_teardown(memcached_st *);
test_return_t libmemcached_string_behavior_test(memcached_st *);
test_return_t libmemcached_string_distribution_test(memcached_st *);
test_return_t md5_run (memcached_st *);
test_return_t memcached_fetch_result_NOT_FOUND(memcached_st *memc);
test_return_t memcached_get_MEMCACHED_ERRNO(memcached_st *);
test_return_t memcached_get_MEMCACHED_NOTFOUND(memcached_st *memc);
test_return_t memcached_get_by_key_MEMCACHED_ERRNO(memcached_st *memc);
test_return_t memcached_get_by_key_MEMCACHED_NOTFOUND(memcached_st *memc);
test_return_t memcached_get_hashkit_test (memcached_st *);
test_return_t memcached_mget_mixed_memcached_get_TEST(memcached_st *memc);
test_return_t memcached_return_t_TEST(memcached_st *memc);
test_return_t memcached_server_cursor_test(memcached_st *memc);
test_return_t memcached_server_remove_test(memcached_st*);
test_return_t memcached_stat_execute_test(memcached_st *memc);
test_return_t mget_end(memcached_st *memc);
test_return_t mget_execute(memcached_st *original_memc);
test_return_t MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH_TEST(memcached_st *original_memc);
test_return_t mget_result_alloc_test(memcached_st *memc);
test_return_t mget_result_function(memcached_st *memc);
test_return_t mget_result_test(memcached_st *memc);
test_return_t mget_test(memcached_st *memc);
test_return_t murmur_avaibility_test (memcached_st *memc);
test_return_t murmur_run (memcached_st *);
test_return_t murmur3_TEST(hashkit_st *);
test_return_t noreply_test(memcached_st *memc);
test_return_t one_at_a_time_run (memcached_st *);
test_return_t ketama_TEST(memcached_st *);
test_return_t output_ketama_weighted_keys(memcached_st *);
test_return_t libmemcached_util_ping_TEST(memcached_st*);
test_return_t prepend_test(memcached_st *memc);
test_return_t quit_test(memcached_st *memc);
test_return_t read_through(memcached_st *memc);
test_return_t regression_bug_(memcached_st*);
test_return_t regression_bug_421108(memcached_st*);
test_return_t regression_bug_434484(memcached_st*);
test_return_t regression_bug_434843(memcached_st*);
test_return_t regression_bug_434843_buffered(memcached_st*);
test_return_t regression_bug_442914(memcached_st*);
test_return_t regression_bug_447342(memcached_st*);
test_return_t regression_bug_463297(memcached_st*);
test_return_t regression_bug_490486(memcached_st*);
test_return_t regression_bug_490520(memcached_st*);
test_return_t regression_bug_581030(memcached_st*);
test_return_t regression_bug_583031(memcached_st*);
test_return_t regression_1021819_TEST(memcached_st*);
test_return_t regression_bug_655423(memcached_st*);
test_return_t regression_bug_854604(memcached_st*);
test_return_t replace_test(memcached_st *memc);
test_return_t result_alloc(memcached_st *memc);
test_return_t result_static(memcached_st *memc);
test_return_t selection_of_namespace_tests(memcached_st *memc);
test_return_t server_sort2_test(memcached_st *ptr);
test_return_t server_sort_test(memcached_st *ptr);
test_return_t server_unsort_test(memcached_st *ptr);
test_return_t set_memory_alloc(memcached_st *memc);
test_return_t set_namespace(memcached_st *memc);
test_return_t set_namespace_and_binary(memcached_st *memc);
test_return_t set_test(memcached_st *memc);
test_return_t set_test2(memcached_st *memc);
test_return_t set_test3(memcached_st *memc);
test_return_t stats_servername_test(memcached_st *memc);
test_return_t test_get_last_disconnect(memcached_st *memc);
test_return_t test_multiple_get_last_disconnect(memcached_st *);
test_return_t test_verbosity(memcached_st *memc);
test_return_t user_supplied_bug10(memcached_st *memc);
test_return_t user_supplied_bug11(memcached_st *memc);
test_return_t user_supplied_bug12(memcached_st *memc);
test_return_t user_supplied_bug13(memcached_st *memc);
test_return_t user_supplied_bug14(memcached_st *memc);
test_return_t user_supplied_bug15(memcached_st *memc);
test_return_t user_supplied_bug16(memcached_st *memc);
test_return_t user_supplied_bug17(memcached_st *memc);
test_return_t user_supplied_bug19(memcached_st *);
test_return_t user_supplied_bug20(memcached_st *memc);
test_return_t user_supplied_bug21(memcached_st *memc);
test_return_t user_supplied_bug4(memcached_st *memc);
test_return_t user_supplied_bug5(memcached_st *memc);
test_return_t user_supplied_bug6(memcached_st *memc);
test_return_t user_supplied_bug7(memcached_st *memc);
test_return_t user_supplied_bug8(memcached_st *);
test_return_t user_supplied_bug9(memcached_st *memc);
test_return_t userdata_test(memcached_st *memc);
test_return_t util_version_test(memcached_st *memc);
test_return_t version_string_test(memcached_st *);
test_return_t wrong_failure_counter_test(memcached_st *memc);
test_return_t wrong_failure_counter_two_test(memcached_st *memc);
test_return_t kill_HUP_TEST(memcached_st *memc);
test_return_t regression_996813_TEST(memcached_st*);
test_return_t regression_994772_TEST(memcached_st*);
test_return_t regression_1009493_TEST(memcached_st*);
test_return_t regression_1048945_TEST(memcached_st*);
test_return_t regression_1067242_TEST(memcached_st*);
test_return_t comparison_operator_memcached_st_and__memcached_return_t_TEST(memcached_st*);
test_return_t regression_bug_1251482(memcached_st*);
