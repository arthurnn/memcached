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

memcached_return_t return_value_based_on_buffering(memcached_st*);

test_return_t pre_behavior_ketama(memcached_st*);
test_return_t pre_behavior_ketama_weighted(memcached_st*);
test_return_t pre_binary(memcached_st*);
test_return_t pre_cork(memcached_st*);
test_return_t pre_cork_and_nonblock(memcached_st*);
test_return_t pre_crc(memcached_st*);
test_return_t pre_hash_fnv1_32(memcached_st*);
test_return_t pre_hash_fnv1_64(memcached_st*);
test_return_t pre_hash_fnv1a_32(memcached_st*);
test_return_t pre_hash_fnv1a_64(memcached_st*);
test_return_t pre_hsieh(memcached_st*);
test_return_t pre_jenkins(memcached_st*);
test_return_t pre_md5(memcached_st*);
test_return_t pre_murmur(memcached_st*);
test_return_t pre_nodelay(memcached_st*);
test_return_t pre_nonblock(memcached_st*);
test_return_t pre_nonblock_binary(memcached_st*);
test_return_t pre_replication(memcached_st*);
test_return_t pre_replication_noblock(memcached_st*);
test_return_t pre_settimer(memcached_st*);
test_return_t pre_unix_socket(memcached_st*);
test_return_t pre_buffer(memcached_st*);
test_return_t memcached_servers_reset_MEMCACHED_DISTRIBUTION_CONSISTENT_SETUP(memcached_st *memc);
test_return_t memcached_servers_reset_MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED_SETUP(memcached_st *memc);
test_return_t memcached_servers_reset_SETUP(memcached_st *memc);
