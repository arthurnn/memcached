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

#if defined(HAVE_LIBUUID) && HAVE_LIBUUID
# include <uuid/uuid.h>
#endif

/*
  Test cases
*/

#include <libmemcached-1.0/memcached.h>
#include "libmemcached/is.h"
#include "libmemcached/server_instance.h"

#include <libhashkit-1.0/hashkit.h>

#include <libtest/memcached.hpp>

#include <cerrno>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include <libtest/server.h>

#include "clients/generator.h"

#define SMALL_STRING_LEN 1024

#include <libtest/test.hpp>

using namespace libtest;

#include <libmemcachedutil-1.0/util.h>

#include "tests/hash_results.h"

#include "tests/libmemcached-1.0/callback_counter.h"
#include "tests/libmemcached-1.0/fetch_all_results.h"
#include "tests/libmemcached-1.0/mem_functions.h"
#include "tests/libmemcached-1.0/setup_and_teardowns.h"
#include "tests/print.h"
#include "tests/debug.h"
#include "tests/memc.hpp"

#define UUID_STRING_MAXLENGTH 36

#include "tests/keys.hpp"

#include "libmemcached/instance.hpp"

static memcached_st * create_single_instance_memcached(const memcached_st *original_memc, const char *options)
{
  /*
    If no options are given, copy over at least the binary flag.
  */
  char options_buffer[1024]= { 0 };
  if (options == NULL)
  {
    if (memcached_is_binary(original_memc))
    {
      snprintf(options_buffer, sizeof(options_buffer), "--BINARY");
    }
  }

  /*
   * I only want to hit _one_ server so I know the number of requests I'm
   * sending in the pipeline.
   */
  const memcached_instance_st * instance= memcached_server_instance_by_position(original_memc, 0);

  char server_string[1024];
  int server_string_length;
  if (instance->type == MEMCACHED_CONNECTION_UNIX_SOCKET)
  {
    if (options)
    {
      server_string_length= snprintf(server_string, sizeof(server_string), "--SOCKET=\"%s\" %s",
                                     memcached_server_name(instance), options);
    }
    else
    {
      server_string_length= snprintf(server_string, sizeof(server_string), "--SOCKET=\"%s\"",
                                     memcached_server_name(instance));
    }
  }
  else
  {
    if (options)
    {
      server_string_length= snprintf(server_string, sizeof(server_string), "--server=%s:%d %s",
                                     memcached_server_name(instance), int(memcached_server_port(instance)),
                                     options);
    }
    else
    {
      server_string_length= snprintf(server_string, sizeof(server_string), "--server=%s:%d",
                                     memcached_server_name(instance), int(memcached_server_port(instance)));
    }
  }

  if (server_string_length <= 0)
  {
    return NULL;
  }

  char errror_buffer[1024];
  if (memcached_failed(libmemcached_check_configuration(server_string, server_string_length, errror_buffer, sizeof(errror_buffer))))
  {
    Error << "Failed to parse (" << server_string << ") " << errror_buffer;
    return NULL;
  }

  return memcached(server_string, server_string_length);
}


test_return_t init_test(memcached_st *not_used)
{
  memcached_st memc;
  (void)not_used;

  (void)memcached_create(&memc);
  memcached_free(&memc);

  return TEST_SUCCESS;
}

#define TEST_PORT_COUNT 7
in_port_t test_ports[TEST_PORT_COUNT];

static memcached_return_t server_display_function(const memcached_st *ptr,
                                                  const memcached_instance_st * server,
                                                  void *context)
{
  /* Do Nothing */
  size_t bigger= *((size_t *)(context));
  (void)ptr;
  fatal_assert(bigger <= memcached_server_port(server));
  *((size_t *)(context))= memcached_server_port(server);

  return MEMCACHED_SUCCESS;
}

static memcached_return_t dump_server_information(const memcached_st *ptr,
                                                  const memcached_instance_st * instance,
                                                  void *context)
{
  /* Do Nothing */
  FILE *stream= (FILE *)context;
  (void)ptr;

  fprintf(stream, "Memcached Server: %s %u Version %u.%u.%u\n",
          memcached_server_name(instance),
          memcached_server_port(instance),
          instance->major_version,
          instance->minor_version,
          instance->micro_version);

  return MEMCACHED_SUCCESS;
}

test_return_t server_sort_test(memcached_st *ptr)
{
  size_t bigger= 0; /* Prime the value for the test_true in server_display_function */

  memcached_return_t rc;
  memcached_server_fn callbacks[1];
  memcached_st *local_memc;
  (void)ptr;

  local_memc= memcached_create(NULL);
  test_true(local_memc);
  memcached_behavior_set(local_memc, MEMCACHED_BEHAVIOR_SORT_HOSTS, 1);

  for (uint32_t x= 0; x < TEST_PORT_COUNT; x++)
  {
    test_ports[x]= (in_port_t)random() % 64000;
    rc= memcached_server_add_with_weight(local_memc, "localhost", test_ports[x], 0);
    test_compare(memcached_server_count(local_memc), x +1);
#if 0 // Rewrite
    test_true(memcached_server_list_count(memcached_server_list(local_memc)) == x+1);
#endif
    test_compare(MEMCACHED_SUCCESS, rc);
  }

  callbacks[0]= server_display_function;
  memcached_server_cursor(local_memc, callbacks, (void *)&bigger,  1);


  memcached_free(local_memc);

  return TEST_SUCCESS;
}

test_return_t server_sort2_test(memcached_st *ptr)
{
  size_t bigger= 0; /* Prime the value for the test_true in server_display_function */
  memcached_server_fn callbacks[1];
  memcached_st *local_memc;
  const memcached_instance_st * instance;
  (void)ptr;

  local_memc= memcached_create(NULL);
  test_true(local_memc);
  test_compare(MEMCACHED_SUCCESS, 
               memcached_behavior_set(local_memc, MEMCACHED_BEHAVIOR_SORT_HOSTS, 1));

  test_compare(MEMCACHED_SUCCESS,
               memcached_server_add_with_weight(local_memc, "MEMCACHED_BEHAVIOR_SORT_HOSTS", 43043, 0));
  instance= memcached_server_instance_by_position(local_memc, 0);
  test_compare(in_port_t(43043), memcached_server_port(instance));

  test_compare(MEMCACHED_SUCCESS,
               memcached_server_add_with_weight(local_memc, "MEMCACHED_BEHAVIOR_SORT_HOSTS", 43042, 0));

  instance= memcached_server_instance_by_position(local_memc, 0);
  test_compare(in_port_t(43042), memcached_server_port(instance));

  instance= memcached_server_instance_by_position(local_memc, 1);
  test_compare(in_port_t(43043), memcached_server_port(instance));

  callbacks[0]= server_display_function;
  memcached_server_cursor(local_memc, callbacks, (void *)&bigger,  1);


  memcached_free(local_memc);

  return TEST_SUCCESS;
}

test_return_t memcached_server_remove_test(memcached_st*)
{
  const char *server_string= "--server=localhost:4444 --server=localhost:4445 --server=localhost:4446 --server=localhost:4447 --server=localhost --server=memcache1.memcache.bk.sapo.pt:11211 --server=memcache1.memcache.bk.sapo.pt:11212 --server=memcache1.memcache.bk.sapo.pt:11213 --server=memcache1.memcache.bk.sapo.pt:11214 --server=memcache2.memcache.bk.sapo.pt:11211 --server=memcache2.memcache.bk.sapo.pt:11212 --server=memcache2.memcache.bk.sapo.pt:11213 --server=memcache2.memcache.bk.sapo.pt:11214";
  char buffer[BUFSIZ];

  test_compare(MEMCACHED_SUCCESS,
               libmemcached_check_configuration(server_string, strlen(server_string), buffer, sizeof(buffer)));
  memcached_st *memc= memcached(server_string, strlen(server_string));
  test_true(memc);

  memcached_server_fn callbacks[1];
  callbacks[0]= server_print_callback;
  memcached_server_cursor(memc, callbacks, NULL,  1);

  memcached_free(memc);

  return TEST_SUCCESS;
}

static memcached_return_t server_display_unsort_function(const memcached_st*,
                                                         const memcached_instance_st * server,
                                                         void *context)
{
  /* Do Nothing */
  uint32_t x= *((uint32_t *)(context));

  if (! (test_ports[x] == memcached_server_port(server)))
  {
    fprintf(stderr, "%lu -> %lu\n", (unsigned long)test_ports[x], (unsigned long)memcached_server_port(server));
    return MEMCACHED_FAILURE;
  }

  *((uint32_t *)(context))= ++x;

  return MEMCACHED_SUCCESS;
}

test_return_t server_unsort_test(memcached_st *ptr)
{
  size_t counter= 0; /* Prime the value for the test_true in server_display_function */
  size_t bigger= 0; /* Prime the value for the test_true in server_display_function */
  memcached_server_fn callbacks[1];
  memcached_st *local_memc;
  (void)ptr;

  local_memc= memcached_create(NULL);
  test_true(local_memc);

  for (uint32_t x= 0; x < TEST_PORT_COUNT; x++)
  {
    test_ports[x]= (in_port_t)(random() % 64000);
    test_compare(MEMCACHED_SUCCESS,
                 memcached_server_add_with_weight(local_memc, "localhost", test_ports[x], 0));
    test_compare(memcached_server_count(local_memc), x +1);
#if 0 // Rewrite
    test_true(memcached_server_list_count(memcached_server_list(local_memc)) == x+1);
#endif
  }

  callbacks[0]= server_display_unsort_function;
  memcached_server_cursor(local_memc, callbacks, (void *)&counter,  1);

  /* Now we sort old data! */
  memcached_behavior_set(local_memc, MEMCACHED_BEHAVIOR_SORT_HOSTS, 1);
  callbacks[0]= server_display_function;
  memcached_server_cursor(local_memc, callbacks, (void *)&bigger,  1);


  memcached_free(local_memc);

  return TEST_SUCCESS;
}

test_return_t allocation_test(memcached_st *not_used)
{
  (void)not_used;
  memcached_st *memc;
  memc= memcached_create(NULL);
  test_true(memc);
  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t clone_test(memcached_st *memc)
{
  /* All null? */
  {
    memcached_st *memc_clone;
    memc_clone= memcached_clone(NULL, NULL);
    test_true(memc_clone);
    memcached_free(memc_clone);
  }

  /* Can we init from null? */
  {
    memcached_st *memc_clone;
    memc_clone= memcached_clone(NULL, memc);
    test_true(memc_clone);

    { // Test allocators
      test_true(memc_clone->allocators.free == memc->allocators.free);
      test_true(memc_clone->allocators.malloc == memc->allocators.malloc);
      test_true(memc_clone->allocators.realloc == memc->allocators.realloc);
      test_true(memc_clone->allocators.calloc == memc->allocators.calloc);
    }

    test_true(memc_clone->connect_timeout == memc->connect_timeout);
    test_true(memc_clone->delete_trigger == memc->delete_trigger);
    test_true(memc_clone->distribution == memc->distribution);
    { // Test all of the flags
      test_true(memc_clone->flags.no_block == memc->flags.no_block);
      test_true(memc_clone->flags.tcp_nodelay == memc->flags.tcp_nodelay);
      test_true(memc_clone->flags.support_cas == memc->flags.support_cas);
      test_true(memc_clone->flags.buffer_requests == memc->flags.buffer_requests);
      test_true(memc_clone->flags.use_sort_hosts == memc->flags.use_sort_hosts);
      test_true(memc_clone->flags.verify_key == memc->flags.verify_key);
      test_true(memc_clone->ketama.weighted_ == memc->ketama.weighted_);
      test_true(memc_clone->flags.binary_protocol == memc->flags.binary_protocol);
      test_true(memc_clone->flags.hash_with_namespace == memc->flags.hash_with_namespace);
      test_true(memc_clone->flags.reply == memc->flags.reply);
      test_true(memc_clone->flags.use_udp == memc->flags.use_udp);
      test_true(memc_clone->flags.auto_eject_hosts == memc->flags.auto_eject_hosts);
      test_true(memc_clone->flags.randomize_replica_read == memc->flags.randomize_replica_read);
    }
    test_true(memc_clone->get_key_failure == memc->get_key_failure);
    test_true(hashkit_compare(&memc_clone->hashkit, &memc->hashkit));
    test_true(memc_clone->io_bytes_watermark == memc->io_bytes_watermark);
    test_true(memc_clone->io_msg_watermark == memc->io_msg_watermark);
    test_true(memc_clone->io_key_prefetch == memc->io_key_prefetch);
    test_true(memc_clone->on_cleanup == memc->on_cleanup);
    test_true(memc_clone->on_clone == memc->on_clone);
    test_true(memc_clone->poll_timeout == memc->poll_timeout);
    test_true(memc_clone->rcv_timeout == memc->rcv_timeout);
    test_true(memc_clone->recv_size == memc->recv_size);
    test_true(memc_clone->retry_timeout == memc->retry_timeout);
    test_true(memc_clone->send_size == memc->send_size);
    test_true(memc_clone->server_failure_limit == memc->server_failure_limit);
    test_true(memc_clone->server_timeout_limit == memc->server_timeout_limit);
    test_true(memc_clone->snd_timeout == memc->snd_timeout);
    test_true(memc_clone->user_data == memc->user_data);

    memcached_free(memc_clone);
  }

  /* Can we init from struct? */
  {
    memcached_st declared_clone;
    memcached_st *memc_clone;
    memset(&declared_clone, 0 , sizeof(memcached_st));
    memc_clone= memcached_clone(&declared_clone, NULL);
    test_true(memc_clone);
    memcached_free(memc_clone);
  }

  /* Can we init from struct? */
  {
    memcached_st declared_clone;
    memcached_st *memc_clone;
    memset(&declared_clone, 0 , sizeof(memcached_st));
    memc_clone= memcached_clone(&declared_clone, memc);
    test_true(memc_clone);
    memcached_free(memc_clone);
  }

  return TEST_SUCCESS;
}

test_return_t userdata_test(memcached_st *memc)
{
  void* foo= NULL;
  test_false(memcached_set_user_data(memc, foo));
  test_true(memcached_get_user_data(memc) == foo);
  test_true(memcached_set_user_data(memc, NULL) == foo);

  return TEST_SUCCESS;
}

test_return_t connection_test(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS,
               memcached_server_add_with_weight(memc, "localhost", 0, 0));

  return TEST_SUCCESS;
}

test_return_t libmemcached_string_behavior_test(memcached_st *)
{
  for (int x= MEMCACHED_BEHAVIOR_NO_BLOCK; x < int(MEMCACHED_BEHAVIOR_MAX); ++x)
  {
    test_true(libmemcached_string_behavior(memcached_behavior_t(x)));
  }
  test_compare(37, int(MEMCACHED_BEHAVIOR_MAX));

  return TEST_SUCCESS;
}

test_return_t libmemcached_string_distribution_test(memcached_st *)
{
  for (int x= MEMCACHED_DISTRIBUTION_MODULA; x < int(MEMCACHED_DISTRIBUTION_CONSISTENT_MAX); ++x)
  {
    test_true(libmemcached_string_distribution(memcached_server_distribution_t(x)));
  }
  test_compare(7, int(MEMCACHED_DISTRIBUTION_CONSISTENT_MAX));

  return TEST_SUCCESS;
}

test_return_t memcached_return_t_TEST(memcached_st *memc)
{
  uint32_t values[] = { 851992627U, 2337886783U, 4109241422U, 4001849190U,
                        982370485U, 1263635348U, 4242906218U, 3829656100U,
                        1891735253U, 334139633U, 2257084983U, 3351789013U,
                        13199785U, 2542027183U, 1097051614U, 199566778U,
                        2748246961U, 2465192557U, 1664094137U, 2405439045U,
                        1842224848U, 692413798U, 3479807801U, 919913813U,
                        4269430871U, 610793021U, 527273862U, 1437122909U,
                        2300930706U, 2943759320U, 674306647U, 2400528935U,
                        54481931U, 4186304426U, 1741088401U, 2979625118U,
                        4159057246U, 3425930182U, 2593724503U,  1868899624U,
                        1769812374U, 2302537950U, 1110330676U, 3365377466U, 
                        1336171666U, 3021258493U, 2334992265U, 3861994737U, 
                        3582734124U, 3365377466U };

  // You have updated the memcache_error messages but not updated docs/tests.
  for (int rc= int(MEMCACHED_SUCCESS); rc < int(MEMCACHED_MAXIMUM_RETURN); ++rc)
  {
    uint32_t hash_val;
    const char *msg=  memcached_strerror(memc, memcached_return_t(rc));
    hash_val= memcached_generate_hash_value(msg, strlen(msg),
                                            MEMCACHED_HASH_JENKINS);
    if (values[rc] != hash_val)
    {
      fprintf(stderr, "\n\nYou have updated memcached_return_t without updating the memcached_return_t_TEST\n");
      fprintf(stderr, "%u, %s, (%u)\n\n", (uint32_t)rc, memcached_strerror(memc, memcached_return_t(rc)), hash_val);
    }
    test_compare(values[rc], hash_val);
  }
  test_compare(49, int(MEMCACHED_MAXIMUM_RETURN));

  return TEST_SUCCESS;
}

test_return_t set_test(memcached_st *memc)
{
  memcached_return_t rc= memcached_set(memc,
                                       test_literal_param("foo"),
                                       test_literal_param("when we sanitize"),
                                       time_t(0), (uint32_t)0);
  test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED);

  return TEST_SUCCESS;
}

test_return_t append_test(memcached_st *memc)
{
  memcached_return_t rc;
  const char *in_value= "we";
  size_t value_length;
  uint32_t flags;

  test_compare(MEMCACHED_SUCCESS,
               memcached_flush(memc, 0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc,
                             test_literal_param(__func__),
                             in_value, strlen(in_value),
                             time_t(0), uint32_t(0)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_append(memc,
                                test_literal_param(__func__),
                                " the", strlen(" the"),
                                time_t(0), uint32_t(0)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_append(memc,
                                test_literal_param(__func__),
                                " people", strlen(" people"),
                                time_t(0), uint32_t(0)));

  char *out_value= memcached_get(memc,
                                 test_literal_param(__func__),
                                 &value_length, &flags, &rc);
  test_memcmp(out_value, "we the people", strlen("we the people"));
  test_compare(strlen("we the people"), value_length);
  test_compare(MEMCACHED_SUCCESS, rc);
  free(out_value);

  return TEST_SUCCESS;
}

test_return_t append_binary_test(memcached_st *memc)
{
  uint32_t store_list[] = { 23, 56, 499, 98, 32847, 0 };

  test_compare(MEMCACHED_SUCCESS,
               memcached_flush(memc, 0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc,
                             test_literal_param(__func__),
                             NULL, 0,
                             time_t(0), uint32_t(0)));

  size_t count= 0;
  for (uint32_t x= 0; store_list[x] ; x++)
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_append(memc,
                         test_literal_param(__func__),
                         (char *)&store_list[x], sizeof(uint32_t),
                         time_t(0), uint32_t(0)));
    count++;
  }

  size_t value_length;
  uint32_t flags;
  memcached_return_t rc;
  uint32_t *value= (uint32_t *)memcached_get(memc,
                                             test_literal_param(__func__),
                                             &value_length, &flags, &rc);
  test_compare(value_length, sizeof(uint32_t) * count);
  test_compare(MEMCACHED_SUCCESS, rc);

  for (uint32_t counter= uint32_t(count), *ptr= value; counter; counter--)
  {
    test_compare(*ptr, store_list[count - counter]);
    ptr++;
  }
  free(value);

  return TEST_SUCCESS;
}

test_return_t memcached_mget_mixed_memcached_get_TEST(memcached_st *memc)
{
  keys_st keys(200);

  for (libtest::vchar_ptr_t::iterator iter= keys.begin();
       iter != keys.end(); 
       ++iter)
  {
    test_compare_hint(MEMCACHED_SUCCESS,
                      memcached_set(memc,
                                    (*iter), 36,
                                    NULL, 0,
                                    time_t(0), uint32_t(0)),
                      memcached_last_error_message(memc));
  }

  for (ptrdiff_t loop= 0; loop < 20; loop++)
  {
    if (random() %2)
    {
      test_compare(MEMCACHED_SUCCESS, 
                   memcached_mget(memc, keys.keys_ptr(), keys.lengths_ptr(), keys.size()));

      memcached_result_st *results= memcached_result_create(memc, NULL);
      test_true(results);

      size_t result_count= 0;
      memcached_return_t rc;
      while (memcached_fetch_result(memc, results, &rc))
      {
        result_count++;
      }
      test_true(keys.size() >= result_count);
    }
    else
    {
      int which_key= random() % int(keys.size());
      size_t value_length;
      uint32_t flags;
      memcached_return_t rc;
      char *out_value= memcached_get(memc, keys.key_at(which_key), keys.length_at(which_key),
                                     &value_length, &flags, &rc);
      if (rc == MEMCACHED_NOTFOUND)
      { } // It is possible that the value has been purged.
      else
      {
        test_compare(MEMCACHED_SUCCESS, rc);
      }
      test_null(out_value);
      test_zero(value_length);
      test_zero(flags);
    }
  }

  return TEST_SUCCESS;
}

test_return_t cas2_test(memcached_st *memc)
{
  const char *keys[]= {"fudge", "son", "food"};
  size_t key_length[]= {5, 3, 4};
  const char *value= "we the people";
  size_t value_length= strlen("we the people");

  test_compare(MEMCACHED_SUCCESS, memcached_flush(memc, 0));

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SUPPORT_CAS, true));

  for (uint32_t x= 0; x < 3; x++)
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, keys[x], key_length[x],
                               keys[x], key_length[x],
                               time_t(50), uint32_t(9)));
  }

  test_compare(MEMCACHED_SUCCESS, 
               memcached_mget(memc, keys, key_length, 3));

  memcached_result_st *results= memcached_result_create(memc, NULL);
  test_true(results);

  memcached_return_t rc;
  results= memcached_fetch_result(memc, results, &rc);
  test_true(results);
  test_true(results->item_cas);
  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(memcached_result_cas(results));

  test_memcmp(value, "we the people", strlen("we the people"));
  test_compare(strlen("we the people"), value_length);
  test_compare(MEMCACHED_SUCCESS, rc);

  memcached_result_free(results);

  return TEST_SUCCESS;
}

test_return_t cas_test(memcached_st *memc)
{
  const char* keys[2] = { __func__, NULL };
  size_t keylengths[2] = { strlen(__func__), 0 };

  memcached_result_st results_obj;

  test_compare(MEMCACHED_SUCCESS, memcached_flush(memc, 0));

  test_skip(true, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SUPPORT_CAS, true));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_set(memc,
                             test_literal_param(__func__),
                             test_literal_param("we the people"),
                             (time_t)0, (uint32_t)0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, keylengths, 1));

  memcached_result_st *results= memcached_result_create(memc, &results_obj);
  test_true(results);

  memcached_return_t rc;
  results= memcached_fetch_result(memc, &results_obj, &rc);
  test_true(results);
  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(memcached_result_cas(results));
  test_memcmp("we the people", memcached_result_value(results), test_literal_param_size("we the people"));
  test_compare(test_literal_param_size("we the people"),
               strlen(memcached_result_value(results)));

  uint64_t cas= memcached_result_cas(results);

#if 0
  results= memcached_fetch_result(memc, &results_obj, &rc);
  test_true(rc == MEMCACHED_END);
  test_true(results == NULL);
#endif

  test_compare(MEMCACHED_SUCCESS,
               memcached_cas(memc,
                             test_literal_param(__func__),
                             test_literal_param("change the value"),
                             0, 0, cas));

  /*
   * The item will have a new cas value, so try to set it again with the old
   * value. This should fail!
   */
  test_compare(MEMCACHED_DATA_EXISTS,
               memcached_cas(memc,
                             test_literal_param(__func__),
                             test_literal_param("change the value"),
                             0, 0, cas));

  memcached_result_free(&results_obj);

  return TEST_SUCCESS;
}


test_return_t prepend_test(memcached_st *memc)
{
  const char *key= "fig";
  const char *value= "people";

  test_compare(MEMCACHED_SUCCESS, 
               memcached_flush(memc, 0));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_set(memc, key, strlen(key),
                             value, strlen(value),
                             time_t(0), uint32_t(0)));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_prepend(memc, key, strlen(key),
                                 "the ", strlen("the "),
                                 time_t(0), uint32_t(0)));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_prepend(memc, key, strlen(key),
                                 "we ", strlen("we "),
                                 time_t(0), uint32_t(0)));

  size_t value_length;
  uint32_t flags;
  memcached_return_t rc;
  char *out_value= memcached_get(memc, key, strlen(key),
                       &value_length, &flags, &rc);
  test_memcmp(out_value, "we the people", strlen("we the people"));
  test_compare(strlen("we the people"), value_length);
  test_compare(MEMCACHED_SUCCESS, rc);
  free(out_value);

  return TEST_SUCCESS;
}

/*
  Set the value, then quit to make sure it is flushed.
  Come back in and test that add fails.
*/
test_return_t memcached_add_SUCCESS_TEST(memcached_st *memc)
{
  memcached_return_t rc;
  test_null(memcached_get(memc, test_literal_param(__func__), NULL, NULL, &rc));
  test_compare(MEMCACHED_NOTFOUND, rc);

  test_compare(MEMCACHED_SUCCESS,
               memcached_add(memc,
                             test_literal_param(__func__),
                             test_literal_param("try something else"),
                             time_t(0), uint32_t(0)));

  return TEST_SUCCESS;
}

test_return_t regression_1067242_TEST(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set(memc,
                                                test_literal_param(__func__), 
                                                test_literal_param("-2"),
                                                0, 0));

  memcached_return_t rc;
  char* value;
  test_true((value= memcached_get(memc, test_literal_param(__func__), NULL, NULL, &rc)));
  test_compare(MEMCACHED_SUCCESS, rc);
  free(value);

  for (size_t x= 0; x < 10; x++)
  {
    uint64_t new_number;
    test_compare(MEMCACHED_CLIENT_ERROR,
                 memcached_increment(memc, 
                                     test_literal_param(__func__), 1, &new_number));
    test_compare(MEMCACHED_CLIENT_ERROR, memcached_last_error(memc));
    test_true((value= memcached_get(memc, test_literal_param(__func__), NULL, NULL, &rc)));
    test_compare(MEMCACHED_SUCCESS, rc);
    free(value);
  }

  return TEST_SUCCESS;
}

/*
  Set the value, then quit to make sure it is flushed.
  Come back in and test that add fails.
*/
test_return_t add_test(memcached_st *memc)
{
  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             test_literal_param("when we sanitize"),
                             time_t(0), uint32_t(0)));

  memcached_quit(memc);

  size_t value_length;
  uint32_t flags;
  memcached_return_t rc;
  char *check_value= memcached_get(memc,
                                   test_literal_param(__func__),
                                   &value_length, &flags, &rc);
  test_memcmp(check_value, "when we sanitize", strlen("when we sanitize"));
  test_compare(test_literal_param_size("when we sanitize"), value_length);
  test_compare(MEMCACHED_SUCCESS, rc);
  free(check_value);

  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL) ? MEMCACHED_DATA_EXISTS : MEMCACHED_NOTSTORED,
               memcached_add(memc,
                             test_literal_param(__func__),
                             test_literal_param("try something else"),
                             time_t(0), uint32_t(0)));

  return TEST_SUCCESS;
}

/*
** There was a problem of leaking filedescriptors in the initial release
** of MacOSX 10.5. This test case triggers the problem. On some Solaris
** systems it seems that the kernel is slow on reclaiming the resources
** because the connects starts to time out (the test doesn't do much
** anyway, so just loop 10 iterations)
*/
test_return_t add_wrapper(memcached_st *memc)
{
  unsigned int max= 10000;
#ifdef __sun
  max= 10;
#endif
#ifdef __APPLE__
  max= 10;
#endif

  for (uint32_t x= 0; x < max; x++)
  {
    add_test(memc);
  }

  return TEST_SUCCESS;
}

test_return_t replace_test(memcached_st *memc)
{
  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             test_literal_param("when we sanitize"),
                             time_t(0), uint32_t(0)));

  test_compare(MEMCACHED_SUCCESS,
               memcached_replace(memc,
                                 test_literal_param(__func__),
                                 test_literal_param("first we insert some data"),
                                 time_t(0), uint32_t(0)));

  return TEST_SUCCESS;
}

test_return_t delete_test(memcached_st *memc)
{
  test_compare(return_value_based_on_buffering(memc), 
               memcached_set(memc, 
                             test_literal_param(__func__),
                             test_literal_param("when we sanitize"),
                             time_t(0), uint32_t(0)));

  test_compare(return_value_based_on_buffering(memc),
               memcached_delete(memc, 
                                test_literal_param(__func__),
                                time_t(0)));

  return TEST_SUCCESS;
}

test_return_t flush_test(memcached_st *memc)
{
  uint64_t query_id= memcached_query_id(memc);
  test_compare(MEMCACHED_SUCCESS,
               memcached_flush(memc, 0));
  test_compare(query_id +1, memcached_query_id(memc));

  return TEST_SUCCESS;
}

static memcached_return_t  server_function(const memcached_st *,
                                           const memcached_instance_st *,
                                           void *)
{
  /* Do Nothing */
  return MEMCACHED_SUCCESS;
}

test_return_t memcached_server_cursor_test(memcached_st *memc)
{
  char context[10];
  strncpy(context, "foo bad", sizeof(context));
  memcached_server_fn callbacks[1];

  callbacks[0]= server_function;
  memcached_server_cursor(memc, callbacks, context,  1);
  return TEST_SUCCESS;
}

test_return_t bad_key_test(memcached_st *memc)
{
  memcached_return_t rc;
  const char *key= "foo bad";
  uint32_t flags;

  uint64_t query_id= memcached_query_id(memc);
  
  // Just skip if we are in binary mode.
  test_skip(false, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  test_compare(query_id, memcached_query_id(memc)); // We should not increase the query_id for memcached_behavior_get()

  memcached_st *memc_clone= memcached_clone(NULL, memc);
  test_true(memc_clone);

  query_id= memcached_query_id(memc_clone);
  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc_clone, MEMCACHED_BEHAVIOR_VERIFY_KEY, true));
  test_compare(query_id, memcached_query_id(memc_clone)); // We should not increase the query_id for memcached_behavior_set()
  ASSERT_TRUE(memcached_behavior_get(memc_clone, MEMCACHED_BEHAVIOR_VERIFY_KEY));

  /* All keys are valid in the binary protocol (except for length) */
  if (memcached_behavior_get(memc_clone, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL) == false)
  {
    uint64_t before_query_id= memcached_query_id(memc_clone);
    {
      size_t string_length;
      char *string= memcached_get(memc_clone, key, strlen(key),
                                  &string_length, &flags, &rc);
      test_compare(MEMCACHED_BAD_KEY_PROVIDED, rc);
      test_zero(string_length);
      test_false(string);
    }
    test_compare(before_query_id +1, memcached_query_id(memc_clone));

    query_id= memcached_query_id(memc_clone);
    test_compare(MEMCACHED_SUCCESS,
                 memcached_behavior_set(memc_clone, MEMCACHED_BEHAVIOR_VERIFY_KEY, false));
    test_compare(query_id, memcached_query_id(memc_clone)); // We should not increase the query_id for memcached_behavior_set()
    {
      size_t string_length;
      char *string= memcached_get(memc_clone, key, strlen(key),
                                  &string_length, &flags, &rc);
      test_compare(MEMCACHED_NOTFOUND, rc);
      test_zero(string_length);
      test_false(string);
    }

    /* Test multi key for bad keys */
    const char *keys[] = { "GoodKey", "Bad Key", "NotMine" };
    size_t key_lengths[] = { 7, 7, 7 };
    query_id= memcached_query_id(memc_clone);
    test_compare(MEMCACHED_SUCCESS, 
                 memcached_behavior_set(memc_clone, MEMCACHED_BEHAVIOR_VERIFY_KEY, true));
    test_compare(query_id, memcached_query_id(memc_clone));

    query_id= memcached_query_id(memc_clone);
    test_compare(MEMCACHED_BAD_KEY_PROVIDED,
                 memcached_mget(memc_clone, keys, key_lengths, 3));
    test_compare(query_id +1, memcached_query_id(memc_clone));

    query_id= memcached_query_id(memc_clone);
    // Grouping keys are not required to follow normal key behaviors
    test_compare(MEMCACHED_SUCCESS,
                 memcached_mget_by_key(memc_clone, "foo daddy", 9, keys, key_lengths, 1));
    test_compare(query_id +1, memcached_query_id(memc_clone));

    /* The following test should be moved to the end of this function when the
       memcached server is updated to allow max size length of the keys in the
       binary protocol
    */
    test_compare(MEMCACHED_SUCCESS, 
                 memcached_callback_set(memc_clone, MEMCACHED_CALLBACK_NAMESPACE, NULL));

    libtest::vchar_t longkey;
    {
      libtest::vchar_t::iterator it= longkey.begin();
      longkey.insert(it, MEMCACHED_MAX_KEY, 'a');
    }

    test_compare(longkey.size(), size_t(MEMCACHED_MAX_KEY));
    {
      size_t string_length;
      // We subtract 1
      test_null(memcached_get(memc_clone, &longkey[0], longkey.size() -1, &string_length, &flags, &rc));
      test_compare(MEMCACHED_NOTFOUND, rc);
      test_zero(string_length);

      test_null(memcached_get(memc_clone, &longkey[0], longkey.size(), &string_length, &flags, &rc));
      test_compare(MEMCACHED_BAD_KEY_PROVIDED, rc);
      test_zero(string_length);
    }
  }

  /* Make sure zero length keys are marked as bad */
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_behavior_set(memc_clone, MEMCACHED_BEHAVIOR_VERIFY_KEY, true));
    size_t string_length;
    char *string= memcached_get(memc_clone, key, 0,
                                &string_length, &flags, &rc);
    test_compare(MEMCACHED_BAD_KEY_PROVIDED, rc);
    test_zero(string_length);
    test_false(string);
  }

  memcached_free(memc_clone);

  return TEST_SUCCESS;
}

#define READ_THROUGH_VALUE "set for me"
static memcached_return_t read_through_trigger(memcached_st *, // memc
                                               char *, // key
                                               size_t, //  key_length,
                                               memcached_result_st *result)
{
  return memcached_result_set_value(result, READ_THROUGH_VALUE, strlen(READ_THROUGH_VALUE));
}

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

test_return_t read_through(memcached_st *memc)
{
  memcached_trigger_key_fn cb= (memcached_trigger_key_fn)read_through_trigger;

  size_t string_length;
  uint32_t flags;
  memcached_return_t rc;
  char *string= memcached_get(memc,
                              test_literal_param(__func__),
                              &string_length, &flags, &rc);

  test_compare(MEMCACHED_NOTFOUND, rc);
  test_false(string_length);
  test_false(string);

  test_compare(MEMCACHED_SUCCESS,
               memcached_callback_set(memc, MEMCACHED_CALLBACK_GET_FAILURE, *(void **)&cb));

  string= memcached_get(memc,
                        test_literal_param(__func__),
                        &string_length, &flags, &rc);

  test_compare(MEMCACHED_SUCCESS, rc);
  test_compare(sizeof(READ_THROUGH_VALUE) -1, string_length);
  test_compare(0, string[sizeof(READ_THROUGH_VALUE) -1]);
  test_strcmp(READ_THROUGH_VALUE, string);
  free(string);

  string= memcached_get(memc,
                        test_literal_param(__func__),
                        &string_length, &flags, &rc);

  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(string);
  test_compare(string_length, sizeof(READ_THROUGH_VALUE) -1);
  test_true(string[sizeof(READ_THROUGH_VALUE) -1] == 0);
  test_strcmp(READ_THROUGH_VALUE, string);
  free(string);

  return TEST_SUCCESS;
}

test_return_t set_test2(memcached_st *memc)
{
  for (uint32_t x= 0; x < 10; x++)
  {
    test_compare(return_value_based_on_buffering(memc),
                 memcached_set(memc,
                               test_literal_param("foo"),
                               test_literal_param("train in the brain"),
                               time_t(0), uint32_t(0)));
  }

  return TEST_SUCCESS;
}

test_return_t set_test3(memcached_st *memc)
{
  size_t value_length= 8191;

  libtest::vchar_t value;
  value.reserve(value_length);
  for (uint32_t x= 0; x < value_length; x++)
  {
    value.push_back(char(x % 127));
  }

  /* The dump test relies on there being at least 32 items in memcached */
  for (uint32_t x= 0; x < 32; x++)
  {
    char key[16];

    snprintf(key, sizeof(key), "foo%u", x);

    uint64_t query_id= memcached_query_id(memc);
    test_compare(return_value_based_on_buffering(memc),
                 memcached_set(memc, key, strlen(key),
                               &value[0], value.size(),
                               time_t(0), uint32_t(0)));
    test_compare(query_id +1, memcached_query_id(memc));
  }

  return TEST_SUCCESS;
}

test_return_t mget_end(memcached_st *memc)
{
  const char *keys[]= { "foo", "foo2" };
  size_t lengths[]= { 3, 4 };
  const char *values[]= { "fjord", "41" };

  // Set foo and foo2
  for (size_t x= 0; x < test_array_length(keys); x++)
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc,
                               keys[x], lengths[x],
                               values[x], strlen(values[x]),
                               time_t(0), uint32_t(0)));
  }

  char *string;
  size_t string_length;
  uint32_t flags;

  // retrieve both via mget
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc,
                              keys, lengths,
                              test_array_length(keys)));

  char key[MEMCACHED_MAX_KEY];
  size_t key_length;
  memcached_return_t rc;

  // this should get both
  for (size_t x= 0; x < test_array_length(keys); x++)
  {
    string= memcached_fetch(memc, key, &key_length, &string_length,
                            &flags, &rc);
    test_compare(MEMCACHED_SUCCESS, rc);
    int val = 0;
    if (key_length == 4)
    {
      val= 1;
    }

    test_compare(string_length, strlen(values[val]));
    test_true(strncmp(values[val], string, string_length) == 0);
    free(string);
  }

  // this should indicate end
  string= memcached_fetch(memc, key, &key_length, &string_length, &flags, &rc);
  test_compare(MEMCACHED_END, rc);
  test_null(string);

  // now get just one
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, lengths, 1));

  string= memcached_fetch(memc, key, &key_length, &string_length, &flags, &rc);
  test_compare(key_length, lengths[0]);
  test_true(strncmp(keys[0], key, key_length) == 0);
  test_compare(string_length, strlen(values[0]));
  test_true(strncmp(values[0], string, string_length) == 0);
  test_compare(MEMCACHED_SUCCESS, rc);
  free(string);

  // this should indicate end
  string= memcached_fetch(memc, key, &key_length, &string_length, &flags, &rc);
  test_compare(MEMCACHED_END, rc);
  test_null(string);

  return TEST_SUCCESS;
}

/* Do not copy the style of this code, I just access hosts to testthis function */
test_return_t stats_servername_test(memcached_st *memc)
{
  memcached_stat_st memc_stat;
  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);

  if (LIBMEMCACHED_WITH_SASL_SUPPORT and memcached_get_sasl_callbacks(memc))
  {
    return TEST_SKIPPED;
  }

  test_compare(MEMCACHED_SUCCESS, memcached_stat_servername(&memc_stat, NULL,
                                                            memcached_server_name(instance),
                                                            memcached_server_port(instance)));

  return TEST_SUCCESS;
}

test_return_t increment_test(memcached_st *memc)
{
  uint64_t new_number;

  test_compare(MEMCACHED_SUCCESS, 
               memcached_set(memc, 
                             test_literal_param("number"),
                             test_literal_param("0"),
                             (time_t)0, (uint32_t)0));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_increment(memc, test_literal_param("number"), 1, &new_number));
  test_compare(uint64_t(1), new_number);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_increment(memc, test_literal_param("number"), 1, &new_number));
  test_compare(uint64_t(2), new_number);

  return TEST_SUCCESS;
}

static test_return_t __increment_with_initial_test(memcached_st *memc, uint64_t initial)
{
  uint64_t new_number;

  test_compare(MEMCACHED_SUCCESS, memcached_flush_buffers(memc));

  if (memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL))
  {
    test_compare(MEMCACHED_SUCCESS, 
                 memcached_increment_with_initial(memc, test_literal_param("number"), 1, initial, 0, &new_number));
    test_compare(new_number, initial);

    test_compare(MEMCACHED_SUCCESS, 
                 memcached_increment_with_initial(memc, test_literal_param("number"), 1, initial, 0, &new_number));
    test_compare(new_number, (initial +1));
  }
  else
  {
    test_compare(MEMCACHED_INVALID_ARGUMENTS, 
                 memcached_increment_with_initial(memc, test_literal_param("number"), 1, initial, 0, &new_number));
  }

  return TEST_SUCCESS;
}

test_return_t increment_with_initial_test(memcached_st *memc)
{
  return __increment_with_initial_test(memc, 0);
}

test_return_t increment_with_initial_999_test(memcached_st *memc)
{
  return __increment_with_initial_test(memc, 999);
}

test_return_t decrement_test(memcached_st *memc)
{
  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             test_literal_param("3"),
                             time_t(0), uint32_t(0)));
  
  // Make sure we flush the value we just set
  test_compare(MEMCACHED_SUCCESS, memcached_flush_buffers(memc));

  uint64_t new_number;
  test_compare(MEMCACHED_SUCCESS,
               memcached_decrement(memc,
                                   test_literal_param(__func__),
                                   1, &new_number));
  test_compare(uint64_t(2), new_number);

  test_compare(MEMCACHED_SUCCESS,
               memcached_decrement(memc,
                                   test_literal_param(__func__),
                                   1, &new_number));
  test_compare(uint64_t(1), new_number);

  return TEST_SUCCESS;
}

static test_return_t __decrement_with_initial_test(memcached_st *memc, uint64_t initial)
{
  test_skip(true, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  test_compare(MEMCACHED_SUCCESS, memcached_flush_buffers(memc));

  uint64_t new_number;
  test_compare(MEMCACHED_SUCCESS,
               memcached_decrement_with_initial(memc,
                                                test_literal_param(__func__),
                                                1, initial, 
                                                0, &new_number));
  test_compare(new_number, initial);

  test_compare(MEMCACHED_SUCCESS,
               memcached_decrement_with_initial(memc,
                                                test_literal_param(__func__),
                                                1, initial, 
                                                0, &new_number));
  test_compare(new_number, (initial - 1));

  return TEST_SUCCESS;
}

test_return_t decrement_with_initial_test(memcached_st *memc)
{
  return __decrement_with_initial_test(memc, 3);
}

test_return_t decrement_with_initial_999_test(memcached_st *memc)
{
  return __decrement_with_initial_test(memc, 999);
}

test_return_t increment_by_key_test(memcached_st *memc)
{
  const char *master_key= "foo";
  const char *key= "number";
  const char *value= "0";

  test_compare(return_value_based_on_buffering(memc),
               memcached_set_by_key(memc, master_key, strlen(master_key),
                                    key, strlen(key),
                                    value, strlen(value),
                                    time_t(0), uint32_t(0)));
  
  // Make sure we flush the value we just set
  test_compare(MEMCACHED_SUCCESS, memcached_flush_buffers(memc));

  uint64_t new_number;
  test_compare(MEMCACHED_SUCCESS,
               memcached_increment_by_key(memc, master_key, strlen(master_key),
                                          key, strlen(key), 1, &new_number));
  test_compare(uint64_t(1), new_number);

  test_compare(MEMCACHED_SUCCESS,
               memcached_increment_by_key(memc, master_key, strlen(master_key),
                                          key, strlen(key), 1, &new_number));
  test_compare(uint64_t(2), new_number);

  return TEST_SUCCESS;
}

test_return_t increment_with_initial_by_key_test(memcached_st *memc)
{
  uint64_t new_number;
  const char *master_key= "foo";
  const char *key= "number";
  uint64_t initial= 0;

  if (memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL))
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_increment_with_initial_by_key(memc, master_key, strlen(master_key),
                                                         key, strlen(key),
                                                         1, initial, 0, &new_number));
    test_compare(new_number, initial);

    test_compare(MEMCACHED_SUCCESS,
                 memcached_increment_with_initial_by_key(memc, master_key, strlen(master_key),
                                                         key, strlen(key),
                                                         1, initial, 0, &new_number));
    test_compare(new_number, (initial +1));
  }
  else
  {
    test_compare(MEMCACHED_INVALID_ARGUMENTS,
                 memcached_increment_with_initial_by_key(memc, master_key, strlen(master_key),
                                                         key, strlen(key),
                                                         1, initial, 0, &new_number));
  }

  return TEST_SUCCESS;
}

test_return_t decrement_by_key_test(memcached_st *memc)
{
  uint64_t new_number;
  const char *value= "3";

  test_compare(return_value_based_on_buffering(memc),
               memcached_set_by_key(memc,
                                    test_literal_param("foo"),
                                    test_literal_param("number"),
                                    value, strlen(value),
                                    (time_t)0, (uint32_t)0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_decrement_by_key(memc,
                                          test_literal_param("foo"),
                                          test_literal_param("number"),
                                          1, &new_number));
  test_compare(uint64_t(2), new_number);

  test_compare(MEMCACHED_SUCCESS,
               memcached_decrement_by_key(memc,
                                          test_literal_param("foo"),
                                          test_literal_param("number"),
                                          1, &new_number));
  test_compare(uint64_t(1), new_number);

  return TEST_SUCCESS;
}

test_return_t decrement_with_initial_by_key_test(memcached_st *memc)
{
  test_skip(true, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  uint64_t new_number;
  uint64_t initial= 3;

  if (memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL))
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_decrement_with_initial_by_key(memc,
                                                         test_literal_param("foo"),
                                                         test_literal_param("number"),
                                                         1, initial, 0, &new_number));
    test_compare(new_number, initial);

    test_compare(MEMCACHED_SUCCESS,
                 memcached_decrement_with_initial_by_key(memc,
                                                         test_literal_param("foo"),
                                                         test_literal_param("number"),
                                                         1, initial, 0, &new_number));
    test_compare(new_number, (initial - 1));
  }
  else
  {
    test_compare(MEMCACHED_INVALID_ARGUMENTS,
                 memcached_decrement_with_initial_by_key(memc,
                                                         test_literal_param("foo"),
                                                         test_literal_param("number"),
                                                         1, initial, 0, &new_number));
  }

  return TEST_SUCCESS;
}
test_return_t binary_increment_with_prefix_test(memcached_st *memc)
{
  test_skip(true, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  test_compare(MEMCACHED_SUCCESS, memcached_callback_set(memc, MEMCACHED_CALLBACK_PREFIX_KEY, (void *)"namespace:"));

  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param("number"),
                             test_literal_param("0"),
                             (time_t)0, (uint32_t)0));

  uint64_t new_number;
  test_compare(MEMCACHED_SUCCESS, memcached_increment(memc, 
                                                      test_literal_param("number"), 
                                                      1, &new_number));
  test_compare(uint64_t(1), new_number);

  test_compare(MEMCACHED_SUCCESS, memcached_increment(memc,
                                                      test_literal_param("number"),
                                                      1, &new_number));
  test_compare(uint64_t(2), new_number);

  return TEST_SUCCESS;
}

test_return_t quit_test(memcached_st *memc)
{
  const char *value= "sanford and sun";

  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             value, strlen(value),
                             time_t(10), uint32_t(3)));
  memcached_quit(memc);

  test_compare(return_value_based_on_buffering(memc),
               memcached_set(memc,
                             test_literal_param(__func__),
                             value, strlen(value),
                             time_t(50), uint32_t(9)));

  return TEST_SUCCESS;
}

test_return_t mget_result_test(memcached_st *memc)
{
  const char *keys[]= {"fudge", "son", "food"};
  size_t key_length[]= {5, 3, 4};

  memcached_result_st results_obj;
  memcached_result_st *results= memcached_result_create(memc, &results_obj);
  test_true(results);
  test_true(&results_obj == results);

  /* We need to empty the server before continueing test */
  test_compare(MEMCACHED_SUCCESS,
               memcached_flush(memc, 0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 3));

  memcached_return_t rc;
  while ((results= memcached_fetch_result(memc, &results_obj, &rc)))
  {
    test_true(results);
  }

  while ((results= memcached_fetch_result(memc, &results_obj, &rc))) { test_true(false); /* We should never see a value returned */ };
  test_false(results);
  test_compare(MEMCACHED_NOTFOUND, rc);

  for (uint32_t x= 0; x < 3; x++)
  {
    rc= memcached_set(memc, keys[x], key_length[x],
                      keys[x], key_length[x],
                      (time_t)50, (uint32_t)9);
    test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED);
  }

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 3));

  while ((results= memcached_fetch_result(memc, &results_obj, &rc)))
  {
    test_true(results);
    test_true(&results_obj == results);
    test_compare(MEMCACHED_SUCCESS, rc);
    test_memcmp(memcached_result_key_value(results),
                memcached_result_value(results),
                memcached_result_length(results));
    test_compare(memcached_result_key_length(results), memcached_result_length(results));
  }

  memcached_result_free(&results_obj);

  return TEST_SUCCESS;
}

test_return_t mget_result_alloc_test(memcached_st *memc)
{
  const char *keys[]= {"fudge", "son", "food"};
  size_t key_length[]= {5, 3, 4};

  memcached_result_st *results;

  /* We need to empty the server before continueing test */
  test_compare(MEMCACHED_SUCCESS,
               memcached_flush(memc, 0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 3));

  memcached_return_t rc;
  while ((results= memcached_fetch_result(memc, NULL, &rc)))
  {
    test_true(results);
  }
  test_false(results);
  test_compare(MEMCACHED_NOTFOUND, rc);

  for (uint32_t x= 0; x < 3; x++)
  {
    rc= memcached_set(memc, keys[x], key_length[x],
                      keys[x], key_length[x],
                      (time_t)50, (uint32_t)9);
    test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED);
  }

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 3));

  uint32_t x= 0;
  while ((results= memcached_fetch_result(memc, NULL, &rc)))
  {
    test_true(results);
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(memcached_result_key_length(results), memcached_result_length(results));
    test_memcmp(memcached_result_key_value(results),
                memcached_result_value(results),
                memcached_result_length(results));
    memcached_result_free(results);
    x++;
  }

  return TEST_SUCCESS;
}

test_return_t mget_result_function(memcached_st *memc)
{
  const char *keys[]= {"fudge", "son", "food"};
  size_t key_length[]= {5, 3, 4};
  size_t counter;
  memcached_execute_fn callbacks[1];

  for (uint32_t x= 0; x < 3; x++)
  {
    test_compare(return_value_based_on_buffering(memc), 
                 memcached_set(memc, keys[x], key_length[x],
                               keys[x], key_length[x],
                               time_t(50), uint32_t(9)));
  }
  test_compare(MEMCACHED_SUCCESS, memcached_flush_buffers(memc));
  memcached_quit(memc);

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 3));

  callbacks[0]= &callback_counter;
  counter= 0;

  test_compare(MEMCACHED_SUCCESS, 
               memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));

  test_compare(size_t(3), counter);

  return TEST_SUCCESS;
}

test_return_t mget_test(memcached_st *memc)
{
  const char *keys[]= {"fudge", "son", "food"};
  size_t key_length[]= {5, 3, 4};

  char return_key[MEMCACHED_MAX_KEY];
  size_t return_key_length;
  char *return_value;
  size_t return_value_length;

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 3));

  uint32_t flags;
  memcached_return_t rc;
  while ((return_value= memcached_fetch(memc, return_key, &return_key_length,
                                        &return_value_length, &flags, &rc)))
  {
    test_true(return_value);
  }
  test_false(return_value);
  test_zero(return_value_length);
  test_compare(MEMCACHED_NOTFOUND, rc);

  for (uint32_t x= 0; x < 3; x++)
  {
    rc= memcached_set(memc, keys[x], key_length[x],
                      keys[x], key_length[x],
                      (time_t)50, (uint32_t)9);
    test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED);
  }
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 3));

  uint32_t x= 0;
  while ((return_value= memcached_fetch(memc, return_key, &return_key_length,
                                        &return_value_length, &flags, &rc)))
  {
    test_true(return_value);
    test_compare(MEMCACHED_SUCCESS, rc);
    if (not memc->_namespace)
    {
      test_compare(return_key_length, return_value_length);
      test_memcmp(return_value, return_key, return_value_length);
    }
    free(return_value);
    x++;
  }

  return TEST_SUCCESS;
}

test_return_t mget_execute(memcached_st *original_memc)
{
  test_skip(true, memcached_behavior_get(original_memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  memcached_st *memc= create_single_instance_memcached(original_memc, "--BINARY-PROTOCOL");
  test_true(memc);

  keys_st keys(20480);

  /* First add all of the items.. */
  char blob[1024] = {0};

  for (size_t x= 0; x < keys.size(); ++x)
  {
    uint64_t query_id= memcached_query_id(memc);
    memcached_return_t rc= memcached_add(memc,
                                         keys.key_at(x), keys.length_at(x),
                                         blob, sizeof(blob),
                                         0, 0);
    ASSERT_TRUE_(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED, "Returned %s", memcached_strerror(NULL, rc));
    test_compare(query_id +1, memcached_query_id(memc));
  }

  /* Try to get all of them with a large multiget */
  size_t counter= 0;
  memcached_execute_fn callbacks[]= { &callback_counter };
  test_compare(MEMCACHED_SUCCESS, 
               memcached_mget_execute(memc,
                                      keys.keys_ptr(), keys.lengths_ptr(),
                                      keys.size(), callbacks, &counter, 1));

  {
    uint64_t query_id= memcached_query_id(memc);
    test_compare(MEMCACHED_SUCCESS, 
                 memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));
    test_compare(query_id, memcached_query_id(memc));

    /* Verify that we got all of the items */
    test_compare(keys.size(), counter);
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH_TEST(memcached_st *original_memc)
{
  test_skip(true, memcached_behavior_get(original_memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL));

  memcached_st *memc= create_single_instance_memcached(original_memc, "--BINARY-PROTOCOL");
  test_true(memc);

  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH, 8));

  keys_st keys(20480);

  /* First add all of the items.. */
  char blob[1024] = {0};

  for (size_t x= 0; x < keys.size(); ++x)
  {
    uint64_t query_id= memcached_query_id(memc);
    memcached_return_t rc= memcached_add(memc,
                                         keys.key_at(x), keys.length_at(x),
                                         blob, sizeof(blob),
                                         0, 0);
    test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED);
    test_compare(query_id +1, memcached_query_id(memc));
  }

  /* Try to get all of them with a large multiget */
  size_t counter= 0;
  memcached_execute_fn callbacks[]= { &callback_counter };
  test_compare(MEMCACHED_SUCCESS, 
               memcached_mget_execute(memc,
                                      keys.keys_ptr(), keys.lengths_ptr(),
                                      keys.size(), callbacks, &counter, 1));

  {
    uint64_t query_id= memcached_query_id(memc);
    test_compare(MEMCACHED_SUCCESS, 
                 memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));
    test_compare(query_id, memcached_query_id(memc));

    /* Verify that we got all of the items */
    test_compare(keys.size(), counter);
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

#define REGRESSION_BINARY_VS_BLOCK_COUNT  20480
static pairs_st *global_pairs= NULL;

test_return_t key_setup(memcached_st *memc)
{
  test_skip(TEST_SUCCESS, pre_binary(memc));

  global_pairs= pairs_generate(REGRESSION_BINARY_VS_BLOCK_COUNT, 0);

  return TEST_SUCCESS;
}

test_return_t key_teardown(memcached_st *)
{
  pairs_free(global_pairs);
  global_pairs= NULL;

  return TEST_SUCCESS;
}

test_return_t block_add_regression(memcached_st *memc)
{
  /* First add all of the items.. */
  for (ptrdiff_t x= 0; x < REGRESSION_BINARY_VS_BLOCK_COUNT; ++x)
  {
    libtest::vchar_t blob;
    libtest::vchar::make(blob, 1024);

    memcached_return_t rc= memcached_add_by_key(memc,
                                                test_literal_param("bob"),
                                                global_pairs[x].key, global_pairs[x].key_length,
                                                &blob[0], blob.size(),
                                                time_t(0), uint32_t(0));
    if (rc == MEMCACHED_MEMORY_ALLOCATION_FAILURE)
    {
      Error << memcached_last_error_message(memc);
      return TEST_SKIPPED;
    }
    test_compare(*memc, MEMCACHED_SUCCESS);
    test_compare(rc, MEMCACHED_SUCCESS);
  }

  return TEST_SUCCESS;
}

test_return_t binary_add_regression(memcached_st *memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, true));
  return block_add_regression(memc);
}

test_return_t get_stats_keys(memcached_st *memc)
{
 char **stat_list;
 char **ptr;
 memcached_stat_st memc_stat;
 memcached_return_t rc;

 stat_list= memcached_stat_get_keys(memc, &memc_stat, &rc);
 test_compare(MEMCACHED_SUCCESS, rc);
 for (ptr= stat_list; *ptr; ptr++)
   test_true(*ptr);

 free(stat_list);

 return TEST_SUCCESS;
}

test_return_t version_string_test(memcached_st *)
{
  test_strcmp(LIBMEMCACHED_VERSION_STRING, memcached_lib_version());

  return TEST_SUCCESS;
}

test_return_t get_stats(memcached_st *memc)
{
 memcached_return_t rc;

 memcached_stat_st *memc_stat= memcached_stat(memc, NULL, &rc);
 test_compare(MEMCACHED_SUCCESS, rc);
 test_true(memc_stat);

 for (uint32_t x= 0; x < memcached_server_count(memc); x++)
 {
   char **stat_list= memcached_stat_get_keys(memc, memc_stat+x, &rc);
   test_compare(MEMCACHED_SUCCESS, rc);
   for (char **ptr= stat_list; *ptr; ptr++) {};

   free(stat_list);
 }

 memcached_stat_free(NULL, memc_stat);

  return TEST_SUCCESS;
}

test_return_t add_host_test(memcached_st *memc)
{
  char servername[]= "0.example.com";

  memcached_return_t rc;
  memcached_server_st *servers= memcached_server_list_append_with_weight(NULL, servername, 400, 0, &rc);
  test_compare(1U, memcached_server_list_count(servers));

  for (unsigned int x= 2; x < 20; x++)
  {
    char buffer[SMALL_STRING_LEN];

    snprintf(buffer, SMALL_STRING_LEN, "%u.example.com", 400+x);
    servers= memcached_server_list_append_with_weight(servers, buffer, 401, 0,
                                     &rc);
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(x, memcached_server_list_count(servers));
  }

  test_compare(MEMCACHED_SUCCESS, memcached_server_push(memc, servers));
  test_compare(MEMCACHED_SUCCESS, memcached_server_push(memc, servers));

  memcached_server_list_free(servers);

  return TEST_SUCCESS;
}

test_return_t regression_1048945_TEST(memcached_st*)
{
  memcached_return status;

  memcached_server_st* list= memcached_server_list_append_with_weight(NULL, "a", 11211, 0, &status);
  test_compare(status, MEMCACHED_SUCCESS);

  list= memcached_server_list_append_with_weight(list, "b", 11211, 0, &status);
  test_compare(status, MEMCACHED_SUCCESS);

  list= memcached_server_list_append_with_weight(list, "c", 11211, 0, &status);
  test_compare(status, MEMCACHED_SUCCESS);

  memcached_st* memc= memcached_create(NULL);

  status= memcached_server_push(memc, list);
  memcached_server_list_free(list);
  test_compare(status, MEMCACHED_SUCCESS);

  const memcached_instance_st * server= memcached_server_by_key(memc, test_literal_param(__func__), &status);
  test_true(server);
  test_compare(status, MEMCACHED_SUCCESS);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t memcached_fetch_result_NOT_FOUND(memcached_st *memc)
{
  memcached_return_t rc;

  const char *key= "not_found";
  size_t key_length= test_literal_param_size("not_found");

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, &key, &key_length, 1));

  memcached_result_st *result= memcached_fetch_result(memc, NULL, &rc);
  test_null(result);
  test_compare(MEMCACHED_NOTFOUND, rc);

  memcached_result_free(result);

  return TEST_SUCCESS;
}

static memcached_return_t  clone_test_callback(memcached_st *, memcached_st *)
{
  return MEMCACHED_SUCCESS;
}

static memcached_return_t  cleanup_test_callback(memcached_st *)
{
  return MEMCACHED_SUCCESS;
}

test_return_t callback_test(memcached_st *memc)
{
  /* Test User Data */
  {
    int x= 5;
    int *test_ptr;
    memcached_return_t rc;

    test_compare(MEMCACHED_SUCCESS, memcached_callback_set(memc, MEMCACHED_CALLBACK_USER_DATA, &x));
    test_ptr= (int *)memcached_callback_get(memc, MEMCACHED_CALLBACK_USER_DATA, &rc);
    test_true(*test_ptr == x);
  }

  /* Test Clone Callback */
  {
    memcached_clone_fn clone_cb= (memcached_clone_fn)clone_test_callback;
    void *clone_cb_ptr= *(void **)&clone_cb;
    void *temp_function= NULL;

    test_compare(MEMCACHED_SUCCESS, memcached_callback_set(memc, MEMCACHED_CALLBACK_CLONE_FUNCTION, clone_cb_ptr));
    memcached_return_t rc;
    temp_function= memcached_callback_get(memc, MEMCACHED_CALLBACK_CLONE_FUNCTION, &rc);
    test_true(temp_function == clone_cb_ptr);
    test_compare(MEMCACHED_SUCCESS, rc);
  }

  /* Test Cleanup Callback */
  {
    memcached_cleanup_fn cleanup_cb= (memcached_cleanup_fn)cleanup_test_callback;
    void *cleanup_cb_ptr= *(void **)&cleanup_cb;
    void *temp_function= NULL;
    memcached_return_t rc;

    test_compare(MEMCACHED_SUCCESS, memcached_callback_set(memc, MEMCACHED_CALLBACK_CLONE_FUNCTION, cleanup_cb_ptr));
    temp_function= memcached_callback_get(memc, MEMCACHED_CALLBACK_CLONE_FUNCTION, &rc);
    test_true(temp_function == cleanup_cb_ptr);
  }

  return TEST_SUCCESS;
}

/* We don't test the behavior itself, we test the switches */
test_return_t behavior_test(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1);
  test_compare(true, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NO_BLOCK));

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, 1);
  test_compare(true, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY));

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, MEMCACHED_HASH_MD5);
  test_compare(uint64_t(MEMCACHED_HASH_MD5), memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_HASH));

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 0);
  test_zero(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NO_BLOCK));

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, 0);
  test_zero(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY));

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, MEMCACHED_HASH_DEFAULT);
  test_compare(uint64_t(MEMCACHED_HASH_DEFAULT), memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_HASH));

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, MEMCACHED_HASH_CRC);
  test_compare(uint64_t(MEMCACHED_HASH_CRC), memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_HASH));

  test_true(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE));

  test_true(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE));

  uint64_t value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS, value +1);
  test_compare((value +1),  memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS));

  return TEST_SUCCESS;
}

test_return_t MEMCACHED_BEHAVIOR_CORK_test(memcached_st *memc)
{
  test_compare(MEMCACHED_DEPRECATED, 
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CORK, true));

  // Platform dependent
#if 0
  bool value= (bool)memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_CORK);
  test_false(value);
#endif

  return TEST_SUCCESS;
}


test_return_t MEMCACHED_BEHAVIOR_TCP_KEEPALIVE_test(memcached_st *memc)
{
  memcached_return_t rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_KEEPALIVE, true);
  test_true(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_NOT_SUPPORTED);

  bool value= (bool)memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_TCP_KEEPALIVE);

  if (memcached_success(rc))
  {
    test_true(value);
  }
  else
  {
    test_false(value);
  }

  return TEST_SUCCESS;
}


test_return_t MEMCACHED_BEHAVIOR_TCP_KEEPIDLE_test(memcached_st *memc)
{
  memcached_return_t rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_KEEPIDLE, true);
  test_true(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_NOT_SUPPORTED);

  bool value= (bool)memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_TCP_KEEPIDLE);

  if (memcached_success(rc))
  {
    test_true(value);
  }
  else
  {
    test_false(value);
  }

  return TEST_SUCCESS;
}

/* Make sure we behave properly if server list has no values */
test_return_t user_supplied_bug4(memcached_st *memc)
{
  const char *keys[]= {"fudge", "son", "food"};
  size_t key_length[]= {5, 3, 4};

  /* Here we free everything before running a bunch of mget tests */
  memcached_servers_reset(memc);


  /* We need to empty the server before continueing test */
  test_compare(MEMCACHED_NO_SERVERS,
               memcached_flush(memc, 0));

  test_compare(MEMCACHED_NO_SERVERS,
               memcached_mget(memc, keys, key_length, 3));

  {
    unsigned int keys_returned;
    memcached_return_t rc;
    test_compare(TEST_SUCCESS, fetch_all_results(memc, keys_returned, rc));
    test_compare(MEMCACHED_NOTFOUND, rc);
    test_zero(keys_returned);
  }

  for (uint32_t x= 0; x < 3; x++)
  {
    test_compare(MEMCACHED_NO_SERVERS,
                 memcached_set(memc, keys[x], key_length[x],
                               keys[x], key_length[x],
                               (time_t)50, (uint32_t)9));
  }

  test_compare(MEMCACHED_NO_SERVERS, 
               memcached_mget(memc, keys, key_length, 3));

  {
    char *return_value;
    char return_key[MEMCACHED_MAX_KEY];
    memcached_return_t rc;
    size_t return_key_length;
    size_t return_value_length;
    uint32_t flags;
    uint32_t x= 0;
    while ((return_value= memcached_fetch(memc, return_key, &return_key_length,
                                          &return_value_length, &flags, &rc)))
    {
      test_true(return_value);
      test_compare(MEMCACHED_SUCCESS, rc);
      test_true(return_key_length == return_value_length);
      test_memcmp(return_value, return_key, return_value_length);
      free(return_value);
      x++;
    }
  }

  return TEST_SUCCESS;
}

#define VALUE_SIZE_BUG5 1048064
test_return_t user_supplied_bug5(memcached_st *memc)
{
  const char *keys[]= {"036790384900", "036790384902", "036790384904", "036790384906"};
  size_t key_length[]=  {strlen("036790384900"), strlen("036790384902"), strlen("036790384904"), strlen("036790384906")};
  char *value;
  size_t value_length;
  uint32_t flags;
  char *insert_data= new (std::nothrow) char[VALUE_SIZE_BUG5];

  for (uint32_t x= 0; x < VALUE_SIZE_BUG5; x++)
  {
    insert_data[x]= (signed char)rand();
  }

  test_compare(MEMCACHED_SUCCESS,
               memcached_flush(memc, 0));

  memcached_return_t rc;
  test_null(memcached_get(memc, keys[0], key_length[0], &value_length, &flags, &rc));
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 4));

  unsigned int count;
  test_compare(TEST_SUCCESS, fetch_all_results(memc, count, rc));
  test_compare(MEMCACHED_NOTFOUND, rc);
  test_zero(count);

  for (uint32_t x= 0; x < 4; x++)
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, keys[x], key_length[x],
                               insert_data, VALUE_SIZE_BUG5,
                               (time_t)0, (uint32_t)0));
  }

  for (uint32_t x= 0; x < 10; x++)
  {
    value= memcached_get(memc, keys[0], key_length[0],
                         &value_length, &flags, &rc);
    test_compare(rc, MEMCACHED_SUCCESS);
    test_true(value);
    ::free(value);

    test_compare(MEMCACHED_SUCCESS,
                 memcached_mget(memc, keys, key_length, 4));

    test_compare(TEST_SUCCESS, fetch_all_results(memc, count));
    test_compare(4U, count);
  }
  delete [] insert_data;

  return TEST_SUCCESS;
}

test_return_t user_supplied_bug6(memcached_st *memc)
{
  const char *keys[]= {"036790384900", "036790384902", "036790384904", "036790384906"};
  size_t key_length[]=  {strlen("036790384900"), strlen("036790384902"), strlen("036790384904"), strlen("036790384906")};
  char return_key[MEMCACHED_MAX_KEY];
  size_t return_key_length;
  char *value;
  size_t value_length;
  uint32_t flags;
  char *insert_data= new (std::nothrow) char[VALUE_SIZE_BUG5];

  for (uint32_t x= 0; x < VALUE_SIZE_BUG5; x++)
  {
    insert_data[x]= (signed char)rand();
  }

  test_compare(MEMCACHED_SUCCESS, memcached_flush(memc, 0));

  test_compare(TEST_SUCCESS, confirm_keys_dont_exist(memc, keys, test_array_length(keys)));

  // We will now confirm that memcached_mget() returns success, but we will
  // then check to make sure that no actual keys are returned.
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 4));

  memcached_return_t rc;
  uint32_t count= 0;
  while ((value= memcached_fetch(memc, return_key, &return_key_length,
                                 &value_length, &flags, &rc)))
  {
    count++;
  }
  test_zero(count);
  test_compare(MEMCACHED_NOTFOUND, rc);

  for (uint32_t x= 0; x < test_array_length(keys); x++)
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, keys[x], key_length[x],
                               insert_data, VALUE_SIZE_BUG5,
                               (time_t)0, (uint32_t)0));
  }
  test_compare(TEST_SUCCESS, confirm_keys_exist(memc, keys, test_array_length(keys)));

  for (uint32_t x= 0; x < 2; x++)
  {
    value= memcached_get(memc, keys[0], key_length[0],
                         &value_length, &flags, &rc);
    test_true(value);
    free(value);

    test_compare(MEMCACHED_SUCCESS,
                 memcached_mget(memc, keys, key_length, 4));
    /* We test for purge of partial complete fetches */
    for (count= 3; count; count--)
    {
      value= memcached_fetch(memc, return_key, &return_key_length,
                             &value_length, &flags, &rc);
      test_compare(MEMCACHED_SUCCESS, rc);
      test_memcmp(value, insert_data, value_length);
      test_true(value_length);
      free(value);
    }
  }
  delete [] insert_data;

  return TEST_SUCCESS;
}

test_return_t user_supplied_bug8(memcached_st *)
{
  memcached_return_t rc;
  memcached_st *mine;
  memcached_st *memc_clone;

  memcached_server_st *servers;
  const char *server_list= "memcache1.memcache.bk.sapo.pt:11211, memcache1.memcache.bk.sapo.pt:11212, memcache1.memcache.bk.sapo.pt:11213, memcache1.memcache.bk.sapo.pt:11214, memcache2.memcache.bk.sapo.pt:11211, memcache2.memcache.bk.sapo.pt:11212, memcache2.memcache.bk.sapo.pt:11213, memcache2.memcache.bk.sapo.pt:11214";

  servers= memcached_servers_parse(server_list);
  test_true(servers);

  mine= memcached_create(NULL);
  rc= memcached_server_push(mine, servers);
  test_compare(MEMCACHED_SUCCESS, rc);
  memcached_server_list_free(servers);

  test_true(mine);
  memc_clone= memcached_clone(NULL, mine);

  memcached_quit(mine);
  memcached_quit(memc_clone);


  memcached_free(mine);
  memcached_free(memc_clone);

  return TEST_SUCCESS;
}

/* Test flag store/retrieve */
test_return_t user_supplied_bug7(memcached_st *memc)
{
  char *insert_data= new (std::nothrow) char[VALUE_SIZE_BUG5];
  test_true(insert_data);

  for (size_t x= 0; x < VALUE_SIZE_BUG5; x++)
  {
    insert_data[x]= (signed char)rand();
  }

  memcached_flush(memc, 0);

  const char *keys= "036790384900";
  size_t key_length=  strlen(keys);
  test_compare(MEMCACHED_SUCCESS, memcached_set(memc, keys, key_length,
                                                insert_data, VALUE_SIZE_BUG5,
                                                time_t(0), 245U));

  memcached_return_t rc;
  size_t value_length;
  uint32_t flags= 0;
  char *value= memcached_get(memc, keys, key_length,
                             &value_length, &flags, &rc);
  test_compare(245U, flags);
  test_true(value);
  free(value);

  test_compare(MEMCACHED_SUCCESS, memcached_mget(memc, &keys, &key_length, 1));

  char return_key[MEMCACHED_MAX_KEY];
  size_t return_key_length;
  flags= 0;
  value= memcached_fetch(memc, return_key, &return_key_length,
                         &value_length, &flags, &rc);
  test_compare(uint32_t(245), flags);
  test_true(value);
  free(value);
  delete [] insert_data;


  return TEST_SUCCESS;
}

test_return_t user_supplied_bug9(memcached_st *memc)
{
  const char *keys[]= {"UDATA:edevil@sapo.pt", "fudge&*@#", "for^#@&$not"};
  size_t key_length[3];
  uint32_t flags;
  unsigned count= 0;

  char return_key[MEMCACHED_MAX_KEY];
  size_t return_key_length;
  char *return_value;
  size_t return_value_length;


  key_length[0]= strlen("UDATA:edevil@sapo.pt");
  key_length[1]= strlen("fudge&*@#");
  key_length[2]= strlen("for^#@&$not");


  for (unsigned int x= 0; x < 3; x++)
  {
    memcached_return_t rc= memcached_set(memc, keys[x], key_length[x],
                                         keys[x], key_length[x],
                                         (time_t)50, (uint32_t)9);
    test_compare(MEMCACHED_SUCCESS, rc);
  }

  memcached_return_t rc= memcached_mget(memc, keys, key_length, 3);
  test_compare(MEMCACHED_SUCCESS, rc);

  /* We need to empty the server before continueing test */
  while ((return_value= memcached_fetch(memc, return_key, &return_key_length,
                                        &return_value_length, &flags, &rc)) != NULL)
  {
    test_true(return_value);
    free(return_value);
    count++;
  }
  test_compare(3U, count);

  return TEST_SUCCESS;
}

/* We are testing with aggressive timeout to get failures */
test_return_t user_supplied_bug10(memcached_st *memc)
{
  test_skip(memc->servers[0].type, MEMCACHED_CONNECTION_TCP);

  size_t value_length= 512;
  unsigned int set= 1;
  memcached_st *mclone= memcached_clone(NULL, memc);

  memcached_behavior_set(mclone, MEMCACHED_BEHAVIOR_NO_BLOCK, set);
  memcached_behavior_set(mclone, MEMCACHED_BEHAVIOR_TCP_NODELAY, set);
  memcached_behavior_set(mclone, MEMCACHED_BEHAVIOR_POLL_TIMEOUT, uint64_t(0));

  libtest::vchar_t value;
  value.reserve(value_length);
  for (uint32_t x= 0; x < value_length; x++)
  {
    value.push_back(char(x % 127));
  }

  for (unsigned int x= 1; x <= 100000; ++x)
  {
    memcached_return_t rc= memcached_set(mclone, 
                                         test_literal_param("foo"),
                                         &value[0], value.size(),
                                         0, 0);

    test_true((rc == MEMCACHED_SUCCESS or rc == MEMCACHED_WRITE_FAILURE or rc == MEMCACHED_BUFFERED or rc == MEMCACHED_TIMEOUT or rc == MEMCACHED_CONNECTION_FAILURE 
               or rc == MEMCACHED_SERVER_TEMPORARILY_DISABLED));

    if (rc == MEMCACHED_WRITE_FAILURE or rc == MEMCACHED_TIMEOUT)
    {
      x--;
    }
  }

  memcached_free(mclone);

  return TEST_SUCCESS;
}

/*
  We are looking failures in the async protocol
*/
test_return_t user_supplied_bug11(memcached_st *memc)
{
  (void)memc;
#ifndef __APPLE__
  test::Memc mclone(memc);

  memcached_behavior_set(&mclone, MEMCACHED_BEHAVIOR_NO_BLOCK, true);
  memcached_behavior_set(&mclone, MEMCACHED_BEHAVIOR_TCP_NODELAY, true);
  memcached_behavior_set(&mclone, MEMCACHED_BEHAVIOR_POLL_TIMEOUT, size_t(-1));

  test_compare(-1, int32_t(memcached_behavior_get(&mclone, MEMCACHED_BEHAVIOR_POLL_TIMEOUT)));

  libtest::vchar_t value;
  value.reserve(512);
  for (unsigned int x= 0; x < 512; x++)
  {
    value.push_back(char(x % 127));
  }

  for (unsigned int x= 1; x <= 100000; ++x)
  {
    memcached_return_t rc= memcached_set(&mclone, test_literal_param("foo"), &value[0], value.size(), 0, 0);
    (void)rc;
  }

#endif

  return TEST_SUCCESS;
}

/*
  Bug found where incr was not returning MEMCACHED_NOTFOUND when object did not exist.
*/
test_return_t user_supplied_bug12(memcached_st *memc)
{
  memcached_return_t rc;
  uint32_t flags;
  size_t value_length;
  char *value;
  uint64_t number_value;

  value= memcached_get(memc, "autoincrement", strlen("autoincrement"),
                       &value_length, &flags, &rc);
  test_null(value);
  test_compare(MEMCACHED_NOTFOUND, rc);

  rc= memcached_increment(memc, "autoincrement", strlen("autoincrement"),
                          1, &number_value);
  test_null(value);
  /* The binary protocol will set the key if it doesn't exist */
  if (memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL) == 1)
  {
    test_compare(MEMCACHED_SUCCESS, rc);
  }
  else
  {
    test_compare(MEMCACHED_NOTFOUND, rc);
  }

  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc, "autoincrement", strlen("autoincrement"), "1", 1, 0, 0));

  value= memcached_get(memc, "autoincrement", strlen("autoincrement"), &value_length, &flags, &rc);
  test_true(value);
  free(value);

  test_compare(MEMCACHED_SUCCESS,
               memcached_increment(memc, "autoincrement", strlen("autoincrement"), 1, &number_value));
  test_compare(2UL, number_value);

  return TEST_SUCCESS;
}

/*
  Bug found where command total one more than MEMCACHED_MAX_BUFFER
  set key34567890 0 0 8169 \r\n is sent followed by buffer of size 8169, followed by 8169
*/
test_return_t user_supplied_bug13(memcached_st *memc)
{
  char key[] = "key34567890";

  char commandFirst[]= "set key34567890 0 0 ";
  char commandLast[] = " \r\n"; /* first line of command sent to server */
  size_t commandLength;

  commandLength = strlen(commandFirst) + strlen(commandLast) + 4; /* 4 is number of characters in size, probably 8196 */

  size_t overflowSize = MEMCACHED_MAX_BUFFER - commandLength;

  for (size_t testSize= overflowSize - 1; testSize < overflowSize + 1; testSize++)
  {
    char *overflow= new (std::nothrow) char[testSize];
    test_true(overflow);

    memset(overflow, 'x', testSize);
    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, key, strlen(key),
                               overflow, testSize, 0, 0));
    delete [] overflow;
  }

  return TEST_SUCCESS;
}


/*
  Test values of many different sizes
  Bug found where command total one more than MEMCACHED_MAX_BUFFER
  set key34567890 0 0 8169 \r\n
  is sent followed by buffer of size 8169, followed by 8169
*/
test_return_t user_supplied_bug14(memcached_st *memc)
{
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, true);

  libtest::vchar_t value;
  value.reserve(18000);
  for (ptrdiff_t x= 0; x < 18000; x++)
  {
    value.push_back((char) (x % 127));
  }

  for (size_t current_length= 1; current_length < value.size(); current_length++)
  {
    memcached_return_t rc= memcached_set(memc, test_literal_param("foo"),
                                         &value[0], current_length,
                                         (time_t)0, (uint32_t)0);
    ASSERT_TRUE_(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED, "Instead got %s", memcached_strerror(NULL, rc));

    size_t string_length;
    uint32_t flags;
    char *string= memcached_get(memc, test_literal_param("foo"),
                                &string_length, &flags, &rc);

    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(string_length, current_length);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%u", uint32_t(string_length));
    test_memcmp(string, &value[0], string_length);

    free(string);
  }

  return TEST_SUCCESS;
}

/*
  Look for zero length value problems
*/
test_return_t user_supplied_bug15(memcached_st *memc)
{
  for (uint32_t x= 0; x < 2; x++)
  {
    memcached_return_t rc= memcached_set(memc, test_literal_param("mykey"),
                                         NULL, 0,
                                         (time_t)0, (uint32_t)0);

    test_compare(MEMCACHED_SUCCESS, rc);

    size_t length;
    uint32_t flags;
    char *value= memcached_get(memc, test_literal_param("mykey"),
                               &length, &flags, &rc);

    test_compare(MEMCACHED_SUCCESS, rc);
    test_false(value);
    test_zero(length);
    test_zero(flags);

    value= memcached_get(memc, test_literal_param("mykey"),
                         &length, &flags, &rc);

    test_compare(MEMCACHED_SUCCESS, rc);
    test_null(value);
    test_zero(length);
    test_zero(flags);
  }

  return TEST_SUCCESS;
}

/* Check the return sizes on FLAGS to make sure it stores 32bit unsigned values correctly */
test_return_t user_supplied_bug16(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_set(memc, test_literal_param("mykey"),
                                                NULL, 0,
                                                (time_t)0, UINT32_MAX));


  size_t length;
  uint32_t flags;
  memcached_return_t rc;
  char *value= memcached_get(memc, test_literal_param("mykey"),
                             &length, &flags, &rc);

  test_compare(MEMCACHED_SUCCESS, rc);
  test_null(value);
  test_zero(length);
  test_compare(flags, UINT32_MAX);

  return TEST_SUCCESS;
}

#if !defined(__sun) && !defined(__OpenBSD__)
/* Check the validity of chinese key*/
test_return_t user_supplied_bug17(memcached_st *memc)
{
  const char *key= "豆瓣";
  const char *value="我们在炎热抑郁的夏天无法停止豆瓣";
  memcached_return_t rc= memcached_set(memc, key, strlen(key),
                                       value, strlen(value),
                                       (time_t)0, 0);

  test_compare(MEMCACHED_SUCCESS, rc);

  size_t length;
  uint32_t flags;
  char *value2= memcached_get(memc, key, strlen(key),
                              &length, &flags, &rc);

  test_compare(length, strlen(value));
  test_compare(MEMCACHED_SUCCESS, rc);
  test_memcmp(value, value2, length);
  free(value2);

  return TEST_SUCCESS;
}
#endif

/*
  From Andrei on IRC
*/

test_return_t user_supplied_bug19(memcached_st *)
{
  memcached_return_t res;

  memcached_st *memc= memcached(test_literal_param("--server=localhost:11311/?100 --server=localhost:11312/?100"));

  const memcached_instance_st * server= memcached_server_by_key(memc, "a", 1, &res);
  test_true(server);

  memcached_free(memc);

  return TEST_SUCCESS;
}

/* CAS test from Andei */
test_return_t user_supplied_bug20(memcached_st *memc)
{
  const char *key= "abc";
  size_t key_len= strlen("abc");

  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SUPPORT_CAS, true));

  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc,
                             test_literal_param("abc"),
                             test_literal_param("foobar"),
                             (time_t)0, (uint32_t)0));

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, &key, &key_len, 1));

  memcached_result_st result_obj;
  memcached_result_st *result= memcached_result_create(memc, &result_obj);
  test_true(result);

  memcached_result_create(memc, &result_obj);
  memcached_return_t status;
  result= memcached_fetch_result(memc, &result_obj, &status);

  test_true(result);
  test_compare(MEMCACHED_SUCCESS, status);

  memcached_result_free(result);

  return TEST_SUCCESS;
}

/* Large mget() of missing keys with binary proto
 *
 * If many binary quiet commands (such as getq's in an mget) fill the output
 * buffer and the server chooses not to respond, memcached_flush hangs. See
 * http://lists.tangent.org/pipermail/libmemcached/2009-August/000918.html
 */

/* sighandler_t function that always asserts false */
static __attribute__((noreturn)) void fail(int)
{
  fatal_assert(0);
}


test_return_t _user_supplied_bug21(memcached_st* memc, size_t key_count)
{
#ifdef WIN32
  (void)memc;
  (void)key_count;
  return TEST_SKIPPED;
#else
  void (*oldalarm)(int);

  memcached_st *memc_clone= memcached_clone(NULL, memc);
  test_true(memc_clone);

  /* only binproto uses getq for mget */
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc_clone, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, true));

  /* empty the cache to ensure misses (hence non-responses) */
  test_compare(MEMCACHED_SUCCESS, memcached_flush(memc_clone, 0));

  keys_st keys(key_count);

  oldalarm= signal(SIGALRM, fail);
  alarm(5);

  test_compare_got(MEMCACHED_SUCCESS,
                   memcached_mget(memc_clone, keys.keys_ptr(), keys.lengths_ptr(), keys.size()),
                   memcached_last_error_message(memc_clone));

  alarm(0);
  signal(SIGALRM, oldalarm);

  memcached_return_t rc;
  uint32_t flags;
  char return_key[MEMCACHED_MAX_KEY];
  size_t return_key_length;
  char *return_value;
  size_t return_value_length;
  while ((return_value= memcached_fetch(memc, return_key, &return_key_length,
                                        &return_value_length, &flags, &rc)))
  {
    test_false(return_value); // There are no keys to fetch, so the value should never be returned
  }
  test_compare(MEMCACHED_NOTFOUND, rc);
  test_zero(return_value_length);
  test_zero(return_key_length);
  test_false(return_key[0]);
  test_false(return_value);

  memcached_free(memc_clone);

  return TEST_SUCCESS;
#endif
}

test_return_t user_supplied_bug21(memcached_st *memc)
{
  test_skip(TEST_SUCCESS, pre_binary(memc));

  /* should work as of r580 */
  test_compare(TEST_SUCCESS,
               _user_supplied_bug21(memc, 10));

  /* should fail as of r580 */
  test_compare(TEST_SUCCESS,
               _user_supplied_bug21(memc, 1000));

  return TEST_SUCCESS;
}

test_return_t comparison_operator_memcached_st_and__memcached_return_t_TEST(memcached_st *)
{
  test::Memc memc_;

  memcached_st *memc= &memc_;

  ASSERT_EQ(memc, MEMCACHED_SUCCESS);
  test_compare(memc, MEMCACHED_SUCCESS);

  ASSERT_NEQ(memc, MEMCACHED_FAILURE);

  return TEST_SUCCESS;
}

test_return_t ketama_TEST(memcached_st *)
{
  test::Memc memc("--server=10.0.1.1:11211 --server=10.0.1.2:11211");

  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(&memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, true));

  test_compare(memcached_behavior_get(&memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED), uint64_t(1));

  test_compare(memcached_behavior_set(&memc, MEMCACHED_BEHAVIOR_KETAMA_HASH, MEMCACHED_HASH_MD5), MEMCACHED_SUCCESS);

  test_compare(memcached_hash_t(memcached_behavior_get(&memc, MEMCACHED_BEHAVIOR_KETAMA_HASH)), MEMCACHED_HASH_MD5);

  test_compare(memcached_behavior_set_distribution(&memc, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY), MEMCACHED_SUCCESS);


  return TEST_SUCCESS;
}

test_return_t output_ketama_weighted_keys(memcached_st *)
{
  memcached_st *memc= memcached_create(NULL);
  test_true(memc);


  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, true));

  uint64_t value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED);
  test_compare(value, uint64_t(1));

  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH, MEMCACHED_HASH_MD5));

  value= memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_HASH);
  test_true(value == MEMCACHED_HASH_MD5);


  test_true(memcached_behavior_set_distribution(memc, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY) == MEMCACHED_SUCCESS);

  memcached_server_st *server_pool;
  server_pool = memcached_servers_parse("10.0.1.1:11211,10.0.1.2:11211,10.0.1.3:11211,10.0.1.4:11211,10.0.1.5:11211,10.0.1.6:11211,10.0.1.7:11211,10.0.1.8:11211,192.168.1.1:11211,192.168.100.1:11211");
  memcached_server_push(memc, server_pool);

  // @todo this needs to be refactored to actually test something.
#if 0
  FILE *fp;
  if ((fp = fopen("ketama_keys.txt", "w")))
  {
    // noop
  } else {
    printf("cannot write to file ketama_keys.txt");
    return TEST_FAILURE;
  }

  for (int x= 0; x < 10000; x++)
  {
    char key[10];
    snprintf(key, sizeof(key), "%d", x);

    uint32_t server_idx = memcached_generate_hash(memc, key, strlen(key));
    char *hostname = memc->hosts[server_idx].hostname;
    in_port_t port = memc->hosts[server_idx].port;
    fprintf(fp, "key %s is on host /%s:%u\n", key, hostname, port);
    const memcached_instance_st * instance=
      memcached_server_instance_by_position(memc, host_index);
  }
  fclose(fp);
#endif
  memcached_server_list_free(server_pool);
  memcached_free(memc);

  return TEST_SUCCESS;
}


test_return_t result_static(memcached_st *memc)
{
  memcached_result_st result;
  memcached_result_st *result_ptr= memcached_result_create(memc, &result);
  test_false(result.options.is_allocated);
  test_true(memcached_is_initialized(&result));
  test_true(result_ptr);
  test_true(result_ptr == &result);

  memcached_result_free(&result);

  test_false(result.options.is_allocated);
  test_false(memcached_is_initialized(&result));

  return TEST_SUCCESS;
}

test_return_t result_alloc(memcached_st *memc)
{
  memcached_result_st *result_ptr= memcached_result_create(memc, NULL);
  test_true(result_ptr);
  test_true(result_ptr->options.is_allocated);
  test_true(memcached_is_initialized(result_ptr));
  memcached_result_free(result_ptr);

  return TEST_SUCCESS;
}


test_return_t add_host_test1(memcached_st *memc)
{
  memcached_return_t rc;
  char servername[]= "0.example.com";

  memcached_server_st *servers= memcached_server_list_append_with_weight(NULL, servername, 400, 0, &rc);
  test_true(servers);
  test_compare(1U, memcached_server_list_count(servers));

  for (uint32_t x= 2; x < 20; x++)
  {
    char buffer[SMALL_STRING_LEN];

    snprintf(buffer, SMALL_STRING_LEN, "%lu.example.com", (unsigned long)(400 +x));
    servers= memcached_server_list_append_with_weight(servers, buffer, 401, 0,
                                                      &rc);
    test_compare(MEMCACHED_SUCCESS, rc);
    test_compare(x, memcached_server_list_count(servers));
  }

  test_compare(MEMCACHED_SUCCESS, memcached_server_push(memc, servers));
  test_compare(MEMCACHED_SUCCESS, memcached_server_push(memc, servers));

  memcached_server_list_free(servers);

  return TEST_SUCCESS;
}


static void my_free(const memcached_st *ptr, void *mem, void *context)
{
  (void)context;
  (void)ptr;
#ifdef HARD_MALLOC_TESTS
  void *real_ptr= (mem == NULL) ? mem : (void*)((caddr_t)mem - 8);
  free(real_ptr);
#else
  free(mem);
#endif
}


static void *my_malloc(const memcached_st *ptr, const size_t size, void *context)
{
  (void)context;
  (void)ptr;
#ifdef HARD_MALLOC_TESTS
  void *ret= malloc(size + 8);
  if (ret != NULL)
  {
    ret= (void*)((caddr_t)ret + 8);
  }
#else
  void *ret= malloc(size);
#endif

  if (ret != NULL)
  {
    memset(ret, 0xff, size);
  }

  return ret;
}


static void *my_realloc(const memcached_st *ptr, void *mem, const size_t size, void *)
{
#ifdef HARD_MALLOC_TESTS
  void *real_ptr= (mem == NULL) ? NULL : (void*)((caddr_t)mem - 8);
  void *nmem= realloc(real_ptr, size + 8);

  void *ret= NULL;
  if (nmem != NULL)
  {
    ret= (void*)((caddr_t)nmem + 8);
  }

  return ret;
#else
  (void)ptr;
  return realloc(mem, size);
#endif
}


static void *my_calloc(const memcached_st *ptr, size_t nelem, const size_t size, void *)
{
#ifdef HARD_MALLOC_TESTS
  void *mem= my_malloc(ptr, nelem * size);
  if (mem)
  {
    memset(mem, 0, nelem * size);
  }

  return mem;
#else
  (void)ptr;
  return calloc(nelem, size);
#endif
}

test_return_t selection_of_namespace_tests(memcached_st *memc)
{
  memcached_return_t rc;
  const char *key= "mine";
  char *value;

  /* Make sure by default none exists */
  value= (char*)memcached_callback_get(memc, MEMCACHED_CALLBACK_NAMESPACE, &rc);
  test_null(value);
  test_compare_got(MEMCACHED_SUCCESS, rc, memcached_strerror(NULL, rc));

  /* Test a clean set */
  test_compare(MEMCACHED_SUCCESS,
               memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, (void *)key));

  value= (char*)memcached_callback_get(memc, MEMCACHED_CALLBACK_NAMESPACE, &rc);
  test_true(value);
  test_memcmp(value, key, strlen(key));
  test_compare_got(MEMCACHED_SUCCESS, rc, memcached_strerror(NULL, rc));

  /* Test that we can turn it off */
  test_compare(MEMCACHED_SUCCESS,
               memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, NULL));

  value= (char*)memcached_callback_get(memc, MEMCACHED_CALLBACK_NAMESPACE, &rc);
  test_null(value);
  test_compare_got(MEMCACHED_SUCCESS, rc, memcached_strerror(NULL, rc));

  /* Now setup for main test */
  test_compare(MEMCACHED_SUCCESS,
               memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, (void *)key));

  value= (char *)memcached_callback_get(memc, MEMCACHED_CALLBACK_NAMESPACE, &rc);
  test_true(value);
  test_compare_got(MEMCACHED_SUCCESS, rc, memcached_strerror(NULL, rc));
  test_memcmp(value, key, strlen(key));

  /* Set to Zero, and then Set to something too large */
  {
    char long_key[255];
    memset(long_key, 0, 255);

    test_compare(MEMCACHED_SUCCESS,
                 memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, NULL));

    ASSERT_NULL_(memcached_callback_get(memc, MEMCACHED_CALLBACK_NAMESPACE, &rc), "Setting namespace to NULL did not work");

    /* Test a long key for failure */
    /* TODO, extend test to determine based on setting, what result should be */
    strncpy(long_key, "Thisismorethentheallottednumberofcharacters", sizeof(long_key));
    test_compare(MEMCACHED_SUCCESS, 
                 memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, long_key));

    /* Now test a key with spaces (which will fail from long key, since bad key is not set) */
    strncpy(long_key, "This is more then the allotted number of characters", sizeof(long_key));
    test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL) ? MEMCACHED_SUCCESS : MEMCACHED_BAD_KEY_PROVIDED,
                 memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, long_key));

    /* Test for a bad prefix, but with a short key */
    test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL) ? MEMCACHED_INVALID_ARGUMENTS : MEMCACHED_SUCCESS,
                 memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_VERIFY_KEY, 1));

    test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL) ? MEMCACHED_SUCCESS : MEMCACHED_BAD_KEY_PROVIDED,
                 memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, "dog cat"));
  }

  return TEST_SUCCESS;
}

test_return_t set_namespace(memcached_st *memc)
{
  memcached_return_t rc;
  const char *key= "mine";

  // Make sure we default to a null namespace
  char* value= (char*)memcached_callback_get(memc, MEMCACHED_CALLBACK_NAMESPACE, &rc);
  ASSERT_NULL_(value, "memc had a value for namespace when none should exist");
  test_compare_got(MEMCACHED_SUCCESS, rc, memcached_strerror(NULL, rc));

  /* Test a clean set */
  test_compare(MEMCACHED_SUCCESS,
               memcached_callback_set(memc, MEMCACHED_CALLBACK_NAMESPACE, (void *)key));

  value= (char*)memcached_callback_get(memc, MEMCACHED_CALLBACK_NAMESPACE, &rc);
  ASSERT_TRUE(value);
  test_memcmp(value, key, strlen(key));
  test_compare_got(MEMCACHED_SUCCESS, rc, memcached_strerror(NULL, rc));

  return TEST_SUCCESS;
}

test_return_t set_namespace_and_binary(memcached_st *memc)
{
  test_return_if(pre_binary(memc));
  test_return_if(set_namespace(memc));

  return TEST_SUCCESS;
}

#ifdef MEMCACHED_ENABLE_DEPRECATED
test_return_t deprecated_set_memory_alloc(memcached_st *memc)
{
  void *test_ptr= NULL;
  void *cb_ptr= NULL;
  {
    memcached_malloc_fn malloc_cb= (memcached_malloc_fn)my_malloc;
    cb_ptr= *(void **)&malloc_cb;
    memcached_return_t rc;

    test_compare(MEMCACHED_SUCCESS,
                 memcached_callback_set(memc, MEMCACHED_CALLBACK_MALLOC_FUNCTION, cb_ptr));
    test_ptr= memcached_callback_get(memc, MEMCACHED_CALLBACK_MALLOC_FUNCTION, &rc);
    test_compare(MEMCACHED_SUCCESS, rc);
    test_true(test_ptr == cb_ptr);
  }

  {
    memcached_realloc_fn realloc_cb=
      (memcached_realloc_fn)my_realloc;
    cb_ptr= *(void **)&realloc_cb;
    memcached_return_t rc;

    test_compare(MEMCACHED_SUCCESS,
                 memcached_callback_set(memc, MEMCACHED_CALLBACK_REALLOC_FUNCTION, cb_ptr));
    test_ptr= memcached_callback_get(memc, MEMCACHED_CALLBACK_REALLOC_FUNCTION, &rc);
    test_compare(MEMCACHED_SUCCESS, rc);
    test_true(test_ptr == cb_ptr);
  }

  {
    memcached_free_fn free_cb=
      (memcached_free_fn)my_free;
    cb_ptr= *(void **)&free_cb;
    memcached_return_t rc;

    test_compare(MEMCACHED_SUCCESS,
                 memcached_callback_set(memc, MEMCACHED_CALLBACK_FREE_FUNCTION, cb_ptr));
    test_ptr= memcached_callback_get(memc, MEMCACHED_CALLBACK_FREE_FUNCTION, &rc);
    test_compare(MEMCACHED_SUCCESS, rc);
    test_true(test_ptr == cb_ptr);
  }

  return TEST_SUCCESS;
}
#endif


test_return_t set_memory_alloc(memcached_st *memc)
{
  test_compare(MEMCACHED_INVALID_ARGUMENTS,
               memcached_set_memory_allocators(memc, NULL, my_free,
                                               my_realloc, my_calloc, NULL));

  test_compare(MEMCACHED_SUCCESS,
               memcached_set_memory_allocators(memc, my_malloc, my_free,
                                               my_realloc, my_calloc, NULL));

  memcached_malloc_fn mem_malloc;
  memcached_free_fn mem_free;
  memcached_realloc_fn mem_realloc;
  memcached_calloc_fn mem_calloc;
  memcached_get_memory_allocators(memc, &mem_malloc, &mem_free,
                                  &mem_realloc, &mem_calloc);

  test_true(mem_malloc == my_malloc);
  test_true(mem_realloc == my_realloc);
  test_true(mem_calloc == my_calloc);
  test_true(mem_free == my_free);

  return TEST_SUCCESS;
}

test_return_t enable_consistent_crc(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT));
  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION),  uint64_t(MEMCACHED_DISTRIBUTION_CONSISTENT));

  test_return_t rc;
  if ((rc= pre_crc(memc)) != TEST_SUCCESS)
  {
    return rc;
  }

  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION),  uint64_t(MEMCACHED_DISTRIBUTION_CONSISTENT));

  if (memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_HASH) != MEMCACHED_HASH_CRC)
  {
    return TEST_SKIPPED;
  }

  return TEST_SUCCESS;
}

test_return_t enable_consistent_hsieh(memcached_st *memc)
{
  test_return_t rc;
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
  if ((rc= pre_hsieh(memc)) != TEST_SUCCESS)
  {
    return rc;
  }

  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION), uint64_t(MEMCACHED_DISTRIBUTION_CONSISTENT));

  if (memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_HASH) != MEMCACHED_HASH_HSIEH)
  {
    return TEST_SKIPPED;
  }

  return TEST_SUCCESS;
}

test_return_t enable_cas(memcached_st *memc)
{
  if (libmemcached_util_version_check(memc, 1, 2, 4))
  {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SUPPORT_CAS, true);

    return TEST_SUCCESS;
  }

  return TEST_SKIPPED;
}

test_return_t check_for_1_2_3(memcached_st *memc)
{
  memcached_version(memc);

  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);

  if ((instance->major_version >= 1 && (instance->minor_version == 2 && instance->micro_version >= 4))
      or instance->minor_version > 2)
  {
    return TEST_SUCCESS;
  }

  return TEST_SKIPPED;
}

test_return_t MEMCACHED_BEHAVIOR_POLL_TIMEOUT_test(memcached_st *memc)
{
  const uint64_t timeout= 100; // Not using, just checking that it sets

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_POLL_TIMEOUT, timeout);

  test_compare(timeout, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_POLL_TIMEOUT));

  return TEST_SUCCESS;
}

test_return_t noreply_test(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, 
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NOREPLY, true));
  test_compare(MEMCACHED_SUCCESS, 
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, true));
  test_compare(MEMCACHED_SUCCESS, 
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SUPPORT_CAS, true));
  test_compare(1LLU, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NOREPLY));
  test_compare(1LLU, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS));
  test_compare(1LLU, memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_SUPPORT_CAS));

  memcached_return_t ret;
  for (int count= 0; count < 5; ++count)
  {
    for (size_t x= 0; x < 100; ++x)
    {
      char key[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
      int check_length= snprintf(key, sizeof(key), "%lu", (unsigned long)x);
      test_false((size_t)check_length >= sizeof(key) || check_length < 0);

      size_t len= (size_t)check_length;

      switch (count)
      {
      case 0:
        ret= memcached_add(memc, key, len, key, len, 0, 0);
        break;
      case 1:
        ret= memcached_replace(memc, key, len, key, len, 0, 0);
        break;
      case 2:
        ret= memcached_set(memc, key, len, key, len, 0, 0);
        break;
      case 3:
        ret= memcached_append(memc, key, len, key, len, 0, 0);
        break;
      case 4:
        ret= memcached_prepend(memc, key, len, key, len, 0, 0);
        break;
      default:
        test_true(count);
        break;
      }
      test_true_got(ret == MEMCACHED_SUCCESS or ret == MEMCACHED_BUFFERED,
                    memcached_strerror(NULL, ret));
    }

    /*
     ** NOTE: Don't ever do this in your code! this is not a supported use of the
     ** API and is _ONLY_ done this way to verify that the library works the
     ** way it is supposed to do!!!!
   */
#if 0
    int no_msg=0;
    for (uint32_t x= 0; x < memcached_server_count(memc); ++x)
    {
      const memcached_instance_st * instance=
        memcached_server_instance_by_position(memc, x);
      no_msg+=(int)(instance->cursor_active);
    }

    test_true(no_msg == 0);
#endif
    test_compare(MEMCACHED_SUCCESS, memcached_flush_buffers(memc));

    /*
     ** Now validate that all items was set properly!
   */
    for (size_t x= 0; x < 100; ++x)
    {
      char key[10];

      int check_length= snprintf(key, sizeof(key), "%lu", (unsigned long)x);

      test_false((size_t)check_length >= sizeof(key) || check_length < 0);

      size_t len= (size_t)check_length;
      size_t length;
      uint32_t flags;
      char* value=memcached_get(memc, key, strlen(key),
                                &length, &flags, &ret);
      // For the moment we will just go to the next key
      if (MEMCACHED_TIMEOUT == ret)
      {
        continue;
      }
      test_true(ret == MEMCACHED_SUCCESS and value != NULL);
      switch (count)
      {
      case 0: /* FALLTHROUGH */
      case 1: /* FALLTHROUGH */
      case 2:
        test_true(strncmp(value, key, len) == 0);
        test_true(len == length);
        break;
      case 3:
        test_true(length == len * 2);
        break;
      case 4:
        test_true(length == len * 3);
        break;
      default:
        test_true(count);
        break;
      }
      free(value);
    }
  }

  /* Try setting an illegal cas value (should not return an error to
   * the caller (because we don't expect a return message from the server)
 */
  const char* keys[]= {"0"};
  size_t lengths[]= {1};
  size_t length;
  uint32_t flags;
  memcached_result_st results_obj;
  memcached_result_st *results;
  test_compare(MEMCACHED_SUCCESS, 
               memcached_mget(memc, keys, lengths, 1));

  results= memcached_result_create(memc, &results_obj);
  test_true(results);
  results= memcached_fetch_result(memc, &results_obj, &ret);
  test_true(results);
  test_compare(MEMCACHED_SUCCESS, ret);
  uint64_t cas= memcached_result_cas(results);
  memcached_result_free(&results_obj);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_cas(memc, keys[0], lengths[0], keys[0], lengths[0], 0, 0, cas));

  /*
   * The item will have a new cas value, so try to set it again with the old
   * value. This should fail!
 */
  test_compare(MEMCACHED_SUCCESS, 
               memcached_cas(memc, keys[0], lengths[0], keys[0], lengths[0], 0, 0, cas));
  test_true(memcached_flush_buffers(memc) == MEMCACHED_SUCCESS);
  char* value=memcached_get(memc, keys[0], lengths[0], &length, &flags, &ret);
  test_true(ret == MEMCACHED_SUCCESS && value != NULL);
  free(value);

  return TEST_SUCCESS;
}

test_return_t analyzer_test(memcached_st *memc)
{
  memcached_analysis_st *report;
  memcached_return_t rc;

  memcached_stat_st *memc_stat= memcached_stat(memc, NULL, &rc);
  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(memc_stat);

  report= memcached_analyze(memc, memc_stat, &rc);
  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(report);

  free(report);
  memcached_stat_free(NULL, memc_stat);

  return TEST_SUCCESS;
}

test_return_t util_version_test(memcached_st *memc)
{
  test_compare(memcached_version(memc), MEMCACHED_SUCCESS);
  test_true(libmemcached_util_version_check(memc, 0, 0, 0));

  bool if_successful= libmemcached_util_version_check(memc, 9, 9, 9);

  // We expect failure
  if (if_successful)
  {
    fprintf(stderr, "\n----------------------------------------------------------------------\n");
    fprintf(stderr, "\nDumping Server Information\n\n");
    memcached_server_fn callbacks[1];

    callbacks[0]= dump_server_information;
    memcached_server_cursor(memc, callbacks, (void *)stderr,  1);
    fprintf(stderr, "\n----------------------------------------------------------------------\n");
  }
  test_true(if_successful == false);

  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);

  memcached_version(memc);

  // We only use one binary when we test, so this should be just fine.
  if_successful= libmemcached_util_version_check(memc, instance->major_version, instance->minor_version, instance->micro_version);
  test_true(if_successful == true);

  if (instance->micro_version > 0)
  {
    if_successful= libmemcached_util_version_check(memc, instance->major_version, instance->minor_version, (uint8_t)(instance->micro_version -1));
  }
  else if (instance->minor_version > 0)
  {
    if_successful= libmemcached_util_version_check(memc, instance->major_version, (uint8_t)(instance->minor_version - 1), instance->micro_version);
  }
  else if (instance->major_version > 0)
  {
    if_successful= libmemcached_util_version_check(memc, (uint8_t)(instance->major_version -1), instance->minor_version, instance->micro_version);
  }

  test_true(if_successful == true);

  if (instance->micro_version > 0)
  {
    if_successful= libmemcached_util_version_check(memc, instance->major_version, instance->minor_version, (uint8_t)(instance->micro_version +1));
  }
  else if (instance->minor_version > 0)
  {
    if_successful= libmemcached_util_version_check(memc, instance->major_version, (uint8_t)(instance->minor_version +1), instance->micro_version);
  }
  else if (instance->major_version > 0)
  {
    if_successful= libmemcached_util_version_check(memc, (uint8_t)(instance->major_version +1), instance->minor_version, instance->micro_version);
  }

  test_true(if_successful == false);

  return TEST_SUCCESS;
}

test_return_t getpid_connection_failure_test(memcached_st *memc)
{
  test_skip(memc->servers[0].type, MEMCACHED_CONNECTION_TCP);
  memcached_return_t rc;
  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);

  // Test both the version that returns a code, and the one that does not.
  test_true(libmemcached_util_getpid(memcached_server_name(instance),
                                     memcached_server_port(instance) -1, NULL) == -1);

  test_true(libmemcached_util_getpid(memcached_server_name(instance),
                                     memcached_server_port(instance) -1, &rc) == -1);
  test_compare_got(MEMCACHED_CONNECTION_FAILURE, rc, memcached_strerror(memc, rc));

  return TEST_SUCCESS;
}


test_return_t getpid_test(memcached_st *memc)
{
  memcached_return_t rc;
  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);

  // Test both the version that returns a code, and the one that does not.
  test_true(libmemcached_util_getpid(memcached_server_name(instance),
                                     memcached_server_port(instance), NULL) > -1);

  test_true(libmemcached_util_getpid(memcached_server_name(instance),
                                     memcached_server_port(instance), &rc) > -1);
  test_compare(MEMCACHED_SUCCESS, rc);

  return TEST_SUCCESS;
}

static memcached_return_t ping_each_server(const memcached_st*,
                                           const memcached_instance_st * instance,
                                           void*)
{
  // Test both the version that returns a code, and the one that does not.
  memcached_return_t rc;
  if (libmemcached_util_ping(memcached_server_name(instance),
                             memcached_server_port(instance), &rc) == false)
  {
    throw libtest::fatal(LIBYATL_DEFAULT_PARAM, "%s:%d %s", memcached_server_name(instance),
                         memcached_server_port(instance), memcached_strerror(NULL, rc));
  }

  if (libmemcached_util_ping(memcached_server_name(instance),
                                   memcached_server_port(instance), NULL) == false)
  {
    throw libtest::fatal(LIBYATL_DEFAULT_PARAM, "%s:%d", memcached_server_name(instance), memcached_server_port(instance));
  }

  return MEMCACHED_SUCCESS;
}

test_return_t libmemcached_util_ping_TEST(memcached_st *memc)
{
  memcached_server_fn callbacks[1]= { ping_each_server };
  memcached_server_cursor(memc, callbacks, NULL,  1);

  return TEST_SUCCESS;
}


#if 0
test_return_t hash_sanity_test (memcached_st *memc)
{
  (void)memc;

  assert(MEMCACHED_HASH_DEFAULT == MEMCACHED_HASH_DEFAULT);
  assert(MEMCACHED_HASH_MD5 == MEMCACHED_HASH_MD5);
  assert(MEMCACHED_HASH_CRC == MEMCACHED_HASH_CRC);
  assert(MEMCACHED_HASH_FNV1_64 == MEMCACHED_HASH_FNV1_64);
  assert(MEMCACHED_HASH_FNV1A_64 == MEMCACHED_HASH_FNV1A_64);
  assert(MEMCACHED_HASH_FNV1_32 == MEMCACHED_HASH_FNV1_32);
  assert(MEMCACHED_HASH_FNV1A_32 == MEMCACHED_HASH_FNV1A_32);
#ifdef HAVE_HSIEH_HASH
  assert(MEMCACHED_HASH_HSIEH == MEMCACHED_HASH_HSIEH);
#endif
  assert(MEMCACHED_HASH_MURMUR == MEMCACHED_HASH_MURMUR);
  assert(MEMCACHED_HASH_JENKINS == MEMCACHED_HASH_JENKINS);
  assert(MEMCACHED_HASH_MAX == MEMCACHED_HASH_MAX);

  return TEST_SUCCESS;
}
#endif

test_return_t hsieh_avaibility_test (memcached_st *memc)
{
  test_skip(true, libhashkit_has_algorithm(HASHKIT_HASH_HSIEH));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH,
                                      (uint64_t)MEMCACHED_HASH_HSIEH));

  return TEST_SUCCESS;
}

test_return_t murmur_avaibility_test (memcached_st *memc)
{
  test_skip(true, libhashkit_has_algorithm(HASHKIT_HASH_MURMUR));

  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, (uint64_t)MEMCACHED_HASH_MURMUR));

  return TEST_SUCCESS;
}

test_return_t one_at_a_time_run (memcached_st *)
{
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(one_at_a_time_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_DEFAULT));
  }

  return TEST_SUCCESS;
}

test_return_t md5_run (memcached_st *)
{
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(md5_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_MD5));
  }

  return TEST_SUCCESS;
}

test_return_t crc_run (memcached_st *)
{
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(crc_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_CRC));
  }

  return TEST_SUCCESS;
}

test_return_t fnv1_64_run (memcached_st *)
{
  test_skip(true, libhashkit_has_algorithm(HASHKIT_HASH_FNV1_64));

  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(fnv1_64_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_FNV1_64));
  }

  return TEST_SUCCESS;
}

test_return_t fnv1a_64_run (memcached_st *)
{
  test_skip(true, libhashkit_has_algorithm(HASHKIT_HASH_FNV1A_64));

  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(fnv1a_64_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_FNV1A_64));
  }

  return TEST_SUCCESS;
}

test_return_t fnv1_32_run (memcached_st *)
{
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(fnv1_32_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_FNV1_32));
  }

  return TEST_SUCCESS;
}

test_return_t fnv1a_32_run (memcached_st *)
{
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(fnv1a_32_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_FNV1A_32));
  }

  return TEST_SUCCESS;
}

test_return_t hsieh_run (memcached_st *)
{
  test_skip(true, libhashkit_has_algorithm(HASHKIT_HASH_HSIEH));

  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(hsieh_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_HSIEH));
  }

  return TEST_SUCCESS;
}

test_return_t murmur_run (memcached_st *)
{
  test_skip(true, libhashkit_has_algorithm(HASHKIT_HASH_MURMUR));

#ifdef WORDS_BIGENDIAN
  (void)murmur_values;
  return TEST_SKIPPED;
#else
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(murmur_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_MURMUR));
  }

  return TEST_SUCCESS;
#endif
}

test_return_t murmur3_TEST(hashkit_st *)
{
  test_skip(true, libhashkit_has_algorithm(HASHKIT_HASH_MURMUR3));

#ifdef WORDS_BIGENDIAN
  (void)murmur3_values;
  return TEST_SKIPPED;
#else
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(murmur3_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_MURMUR3));
  }

  return TEST_SUCCESS;
#endif
}

test_return_t jenkins_run (memcached_st *)
{
  uint32_t x;
  const char **ptr;

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    test_compare(jenkins_values[x],
                 memcached_generate_hash_value(*ptr, strlen(*ptr), MEMCACHED_HASH_JENKINS));
  }

  return TEST_SUCCESS;
}

static uint32_t hash_md5_test_function(const char *string, size_t string_length, void *)
{
  return libhashkit_md5(string, string_length);
}

static uint32_t hash_crc_test_function(const char *string, size_t string_length, void *)
{
  return libhashkit_crc32(string, string_length);
}

test_return_t memcached_get_hashkit_test (memcached_st *)
{
  uint32_t x;
  const char **ptr;
  hashkit_st new_kit;

  memcached_st *memc= memcached(test_literal_param("--server=localhost:1 --server=localhost:2 --server=localhost:3 --server=localhost:4 --server=localhost5 --DISTRIBUTION=modula"));

  uint32_t md5_hosts[]= {4U, 1U, 0U, 1U, 4U, 2U, 0U, 3U, 0U, 0U, 3U, 1U, 0U, 0U, 1U, 3U, 0U, 0U, 0U, 3U, 1U, 0U, 4U, 4U, 3U};
  uint32_t crc_hosts[]= {2U, 4U, 1U, 0U, 2U, 4U, 4U, 4U, 1U, 2U, 3U, 4U, 3U, 4U, 1U, 3U, 3U, 2U, 0U, 0U, 0U, 1U, 2U, 4U, 0U};

  const hashkit_st *kit= memcached_get_hashkit(memc);

  hashkit_clone(&new_kit, kit);
  test_compare(HASHKIT_SUCCESS, hashkit_set_custom_function(&new_kit, hash_md5_test_function, NULL));

  memcached_set_hashkit(memc, &new_kit);

  /*
    Verify Setting the hash.
  */
  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    uint32_t hash_val;

    hash_val= hashkit_digest(kit, *ptr, strlen(*ptr));
    test_compare_got(md5_values[x], hash_val, *ptr);
  }


  /*
    Now check memcached_st.
  */
  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    uint32_t hash_val;

    hash_val= memcached_generate_hash(memc, *ptr, strlen(*ptr));
    test_compare_got(md5_hosts[x], hash_val, *ptr);
  }

  test_compare(HASHKIT_SUCCESS, hashkit_set_custom_function(&new_kit, hash_crc_test_function, NULL));

  memcached_set_hashkit(memc, &new_kit);

  /*
    Verify Setting the hash.
  */
  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    uint32_t hash_val;

    hash_val= hashkit_digest(kit, *ptr, strlen(*ptr));
    test_true(crc_values[x] == hash_val);
  }

  for (ptr= list_to_hash, x= 0; *ptr; ptr++, x++)
  {
    uint32_t hash_val;

    hash_val= memcached_generate_hash(memc, *ptr, strlen(*ptr));
    test_compare(crc_hosts[x], hash_val);
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

/*
  Test case adapted from John Gorman <johngorman2@gmail.com>

  We are testing the error condition when we connect to a server via memcached_get()
  but find that the server is not available.
*/
test_return_t memcached_get_MEMCACHED_ERRNO(memcached_st *)
{
  size_t len;
  uint32_t flags;
  memcached_return rc;

  // Create a handle.
  memcached_st *tl_memc_h= memcached(test_literal_param("--server=localhost:9898 --server=localhost:9899")); // This server should not exist

  // See if memcached is reachable.
  char *value= memcached_get(tl_memc_h, 
                             test_literal_param(__func__),
                             &len, &flags, &rc);

  test_false(value);
  test_zero(len);
  test_true(memcached_failed(rc));

  memcached_free(tl_memc_h);

  return TEST_SUCCESS;
}

/*
  We connect to a server which exists, but search for a key that does not exist.
*/
test_return_t memcached_get_MEMCACHED_NOTFOUND(memcached_st *memc)
{
  size_t len;
  uint32_t flags;
  memcached_return rc;

  // See if memcached is reachable.
  char *value= memcached_get(memc,
                             test_literal_param(__func__),
                             &len, &flags, &rc);

  test_false(value);
  test_zero(len);
  test_compare(MEMCACHED_NOTFOUND, rc);

  return TEST_SUCCESS;
}

/*
  Test case adapted from John Gorman <johngorman2@gmail.com>

  We are testing the error condition when we connect to a server via memcached_get_by_key()
  but find that the server is not available.
*/
test_return_t memcached_get_by_key_MEMCACHED_ERRNO(memcached_st *)
{
  size_t len;
  uint32_t flags;
  memcached_return rc;

  // Create a handle.
  memcached_st *tl_memc_h= memcached_create(NULL);
  memcached_server_st *servers= memcached_servers_parse("localhost:9898,localhost:9899"); // This server should not exist
  memcached_server_push(tl_memc_h, servers);
  memcached_server_list_free(servers);

  // See if memcached is reachable.
  char *value= memcached_get_by_key(tl_memc_h, 
                                    test_literal_param(__func__), // Key
                                    test_literal_param(__func__), // Value
                                    &len, &flags, &rc);

  test_false(value);
  test_zero(len);
  test_true(memcached_failed(rc));

  memcached_free(tl_memc_h);

  return TEST_SUCCESS;
}

/*
  We connect to a server which exists, but search for a key that does not exist.
*/
test_return_t memcached_get_by_key_MEMCACHED_NOTFOUND(memcached_st *memc)
{
  size_t len;
  uint32_t flags;
  memcached_return rc;

  // See if memcached is reachable.
  char *value= memcached_get_by_key(memc, 
                                    test_literal_param(__func__), // Key
                                    test_literal_param(__func__), // Value
                                    &len, &flags, &rc);

  test_false(value);
  test_zero(len);
  test_compare(MEMCACHED_NOTFOUND, rc);

  return TEST_SUCCESS;
}

test_return_t regression_bug_434484(memcached_st *memc)
{
  test_skip(TEST_SUCCESS, pre_binary(memc));

  test_compare(MEMCACHED_NOTSTORED, 
               memcached_append(memc, 
                                test_literal_param(__func__), // Key
                                test_literal_param(__func__), // Value
                                0, 0));

  libtest::vchar_t data;
  data.resize(2048 * 1024);
  test_compare(MEMCACHED_E2BIG,
               memcached_set(memc, 
                             test_literal_param(__func__), // Key
                             &data[0], data.size(), 0, 0));

  return TEST_SUCCESS;
}

test_return_t regression_bug_434843(memcached_st *original_memc)
{
  test_skip(TEST_SUCCESS, pre_binary(original_memc));

  memcached_return_t rc;
  size_t counter= 0;
  memcached_execute_fn callbacks[]= { &callback_counter };

  /*
   * I only want to hit only _one_ server so I know the number of requests I'm
   * sending in the pipleine to the server. Let's try to do a multiget of
   * 1024 (that should satisfy most users don't you think?). Future versions
   * will include a mget_execute function call if you need a higher number.
 */
  memcached_st *memc= create_single_instance_memcached(original_memc, "--BINARY-PROTOCOL");

  keys_st keys(1024);

  /*
   * Run two times.. the first time we should have 100% cache miss,
   * and the second time we should have 100% cache hits
 */
  for (ptrdiff_t y= 0; y < 2; y++)
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_mget(memc, keys.keys_ptr(), keys.lengths_ptr(), keys.size()));

    // One the first run we should get a NOT_FOUND, but on the second some data
    // should be returned.
    test_compare(y ?  MEMCACHED_SUCCESS : MEMCACHED_NOTFOUND, 
                 memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));

    if (y == 0)
    {
      /* The first iteration should give me a 100% cache miss. verify that*/
      char blob[1024]= { 0 };

      test_false(counter);

      for (size_t x= 0; x < keys.size(); ++x)
      {
        rc= memcached_add(memc, 
                          keys.key_at(x), keys.length_at(x),
                          blob, sizeof(blob), 0, 0);
        test_true(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
      }
    }
    else
    {
      /* Verify that we received all of the key/value pairs */
      test_compare(counter, keys.size());
    }
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t regression_bug_434843_buffered(memcached_st *memc)
{
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, true));

  return regression_bug_434843(memc);
}

test_return_t regression_bug_421108(memcached_st *memc)
{
  memcached_return_t rc;
  memcached_stat_st *memc_stat= memcached_stat(memc, NULL, &rc);
  test_compare(MEMCACHED_SUCCESS, rc);

  char *bytes_str= memcached_stat_get_value(memc, memc_stat, "bytes", &rc);
  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(bytes_str);
  char *bytes_read_str= memcached_stat_get_value(memc, memc_stat,
                                                 "bytes_read", &rc);
  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(bytes_read_str);

  char *bytes_written_str= memcached_stat_get_value(memc, memc_stat,
                                                    "bytes_written", &rc);
  test_compare(MEMCACHED_SUCCESS, rc);
  test_true(bytes_written_str);

  unsigned long long bytes= strtoull(bytes_str, 0, 10);
  unsigned long long bytes_read= strtoull(bytes_read_str, 0, 10);
  unsigned long long bytes_written= strtoull(bytes_written_str, 0, 10);

  test_true(bytes != bytes_read);
  test_true(bytes != bytes_written);

  /* Release allocated resources */
  free(bytes_str);
  free(bytes_read_str);
  free(bytes_written_str);
  memcached_stat_free(NULL, memc_stat);

  return TEST_SUCCESS;
}

/*
 * The test case isn't obvious so I should probably document why
 * it works the way it does. Bug 442914 was caused by a bug
 * in the logic in memcached_purge (it did not handle the case
 * where the number of bytes sent was equal to the watermark).
 * In this test case, create messages so that we hit that case
 * and then disable noreply mode and issue a new command to
 * verify that it isn't stuck. If we change the format for the
 * delete command or the watermarks, we need to update this
 * test....
 */
test_return_t regression_bug_442914(memcached_st *original_memc)
{
  test_skip(original_memc->servers[0].type, MEMCACHED_CONNECTION_TCP);

  memcached_st* memc= create_single_instance_memcached(original_memc, "--NOREPLY --TCP-NODELAY");

  for (uint32_t x= 0; x < 250; ++x)
  {
    char key[250];
    size_t len= (size_t)snprintf(key, sizeof(key), "%0250u", x);
    memcached_return_t rc= memcached_delete(memc, key, len, 0);
    char error_buffer[2048]= { 0 };
    snprintf(error_buffer, sizeof(error_buffer), "%s key: %s", memcached_last_error_message(memc), key);
    test_true_got(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED, error_buffer);
  }

  // Delete, and then delete again to look for not found
  {
    char key[250];
    size_t len= snprintf(key, sizeof(key), "%037u", 251U);
    memcached_return_t rc= memcached_delete(memc, key, len, 0);
    test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED);

    test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NOREPLY, false));
    test_compare(MEMCACHED_NOTFOUND, memcached_delete(memc, key, len, 0));
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t regression_bug_447342(memcached_st *memc)
{
  if (memcached_server_count(memc) < 3 or pre_replication(memc) != TEST_SUCCESS)
  {
    return TEST_SKIPPED;
  }

  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS, 2));

  keys_st keys(100);

  for (size_t x= 0; x < keys.size(); ++x)
  {
    test_compare(MEMCACHED_SUCCESS,
                 memcached_set(memc, 
                               keys.key_at(x), keys.length_at(x), // Keys
                               keys.key_at(x), keys.length_at(x), // Values
                               0, 0));
  }

  /*
   ** We are using the quiet commands to store the replicas, so we need
   ** to ensure that all of them are processed before we can continue.
   ** In the test we go directly from storing the object to trying to
   ** receive the object from all of the different servers, so we
   ** could end up in a race condition (the memcached server hasn't yet
   ** processed the quiet command from the replication set when it process
   ** the request from the other client (created by the clone)). As a
   ** workaround for that we call memcached_quit to send the quit command
   ** to the server and wait for the response ;-) If you use the test code
   ** as an example for your own code, please note that you shouldn't need
   ** to do this ;-)
 */
  memcached_quit(memc);

  /* Verify that all messages are stored, and we didn't stuff too much
   * into the servers
 */
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, 
                              keys.keys_ptr(), keys.lengths_ptr(), keys.size()));

  unsigned int counter= 0;
  memcached_execute_fn callbacks[]= { &callback_counter };
  test_compare(MEMCACHED_SUCCESS, 
               memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));

  /* Verify that we received all of the key/value pairs */
  test_compare(counter, keys.size());

  memcached_quit(memc);
  /*
   * Don't do the following in your code. I am abusing the internal details
   * within the library, and this is not a supported interface.
   * This is to verify correct behavior in the library. Fake that two servers
   * are dead..
 */
  const memcached_instance_st * instance_one= memcached_server_instance_by_position(memc, 0);
  const memcached_instance_st * instance_two= memcached_server_instance_by_position(memc, 2);
  in_port_t port0= instance_one->port();
  in_port_t port2= instance_two->port();

  ((memcached_server_write_instance_st)instance_one)->port(0);
  ((memcached_server_write_instance_st)instance_two)->port(0);

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, 
                              keys.keys_ptr(), keys.lengths_ptr(), keys.size()));

  counter= 0;
  test_compare(MEMCACHED_SUCCESS, 
               memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));
  test_compare(counter, keys.size());

  /* restore the memc handle */
  ((memcached_server_write_instance_st)instance_one)->port(port0);
  ((memcached_server_write_instance_st)instance_two)->port(port2);

  memcached_quit(memc);

  /* Remove half of the objects */
  for (size_t x= 0; x < keys.size(); ++x)
  {
    if (x & 1)
    {
      test_compare(MEMCACHED_SUCCESS,
                   memcached_delete(memc, keys.key_at(x), keys.length_at(x), 0));
    }
  }

  memcached_quit(memc);
  ((memcached_server_write_instance_st)instance_one)->port(0);
  ((memcached_server_write_instance_st)instance_two)->port(0);

  /* now retry the command, this time we should have cache misses */
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc,
                              keys.keys_ptr(), keys.lengths_ptr(), keys.size()));

  counter= 0;
  test_compare(MEMCACHED_SUCCESS, 
               memcached_fetch_execute(memc, callbacks, (void *)&counter, 1));
  test_compare(counter, (unsigned int)(keys.size() >> 1));

  /* restore the memc handle */
  ((memcached_server_write_instance_st)instance_one)->port(port0);
  ((memcached_server_write_instance_st)instance_two)->port(port2);

  return TEST_SUCCESS;
}

test_return_t regression_bug_463297(memcached_st *memc)
{
  test_compare(MEMCACHED_INVALID_ARGUMENTS, memcached_delete(memc, "foo", 3, 1));

  // Since we blocked timed delete, this test is no longer valid.
#if 0
  memcached_st *memc_clone= memcached_clone(NULL, memc);
  test_true(memc_clone);
  test_true(memcached_version(memc_clone) == MEMCACHED_SUCCESS);

  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc_clone, 0);

  if (instance->major_version > 1 ||
      (instance->major_version == 1 &&
       instance->minor_version > 2))
  {
    /* Binary protocol doesn't support deferred delete */
    memcached_st *bin_clone= memcached_clone(NULL, memc);
    test_true(bin_clone);
    test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(bin_clone, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1));
    test_compare(MEMCACHED_INVALID_ARGUMENTS, memcached_delete(bin_clone, "foo", 3, 1));
    memcached_free(bin_clone);

    memcached_quit(memc_clone);

    /* If we know the server version, deferred delete should fail
     * with invalid arguments */
    test_compare(MEMCACHED_INVALID_ARGUMENTS, memcached_delete(memc_clone, "foo", 3, 1));

    /* If we don't know the server version, we should get a protocol error */
    memcached_return_t rc= memcached_delete(memc, "foo", 3, 1);

    /* but there is a bug in some of the memcached servers (1.4) that treats
     * the counter as noreply so it doesn't send the proper error message
   */
    test_true_got(rc == MEMCACHED_PROTOCOL_ERROR || rc == MEMCACHED_NOTFOUND || rc == MEMCACHED_CLIENT_ERROR || rc == MEMCACHED_INVALID_ARGUMENTS, memcached_strerror(NULL, rc));

    /* And buffered mode should be disabled and we should get protocol error */
    test_true(memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 1) == MEMCACHED_SUCCESS);
    rc= memcached_delete(memc, "foo", 3, 1);
    test_true_got(rc == MEMCACHED_PROTOCOL_ERROR || rc == MEMCACHED_NOTFOUND || rc == MEMCACHED_CLIENT_ERROR || rc == MEMCACHED_INVALID_ARGUMENTS, memcached_strerror(NULL, rc));

    /* Same goes for noreply... */
    test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NOREPLY, 1));
    rc= memcached_delete(memc, "foo", 3, 1);
    test_true_got(rc == MEMCACHED_PROTOCOL_ERROR || rc == MEMCACHED_NOTFOUND || rc == MEMCACHED_CLIENT_ERROR || rc == MEMCACHED_INVALID_ARGUMENTS, memcached_strerror(NULL, rc));

    /* but a normal request should go through (and be buffered) */
    test_compare(MEMCACHED_BUFFERED, (rc= memcached_delete(memc, "foo", 3, 0)));
    test_compare(MEMCACHED_SUCCESS, memcached_flush_buffers(memc));

    test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 0));
    /* unbuffered noreply should be success */
    test_compare(MEMCACHED_SUCCESS, memcached_delete(memc, "foo", 3, 0));
    /* unbuffered with reply should be not found... */
    test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NOREPLY, 0));
    test_compare(MEMCACHED_NOTFOUND, memcached_delete(memc, "foo", 3, 0));
  }

  memcached_free(memc_clone);
#endif

  return TEST_SUCCESS;
}


/* Test memcached_server_get_last_disconnect
 * For a working server set, shall be NULL
 * For a set of non existing server, shall not be NULL
 */
test_return_t test_get_last_disconnect(memcached_st *memc)
{
  memcached_return_t rc;
  const memcached_instance_st * disconnected_server;

  /* With the working set of server */
  const char *key= "marmotte";
  const char *value= "milka";

  memcached_reset_last_disconnected_server(memc);
  test_false(memc->last_disconnected_server);
  rc= memcached_set(memc, key, strlen(key),
                    value, strlen(value),
                    (time_t)0, (uint32_t)0);
  test_true(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);

  disconnected_server = memcached_server_get_last_disconnect(memc);
  test_false(disconnected_server);

  /* With a non existing server */
  memcached_st *mine;
  memcached_server_st *servers;

  const char *server_list= "localhost:9";

  servers= memcached_servers_parse(server_list);
  test_true(servers);
  mine= memcached_create(NULL);
  rc= memcached_server_push(mine, servers);
  test_compare(MEMCACHED_SUCCESS, rc);
  memcached_server_list_free(servers);
  test_true(mine);

  rc= memcached_set(mine, key, strlen(key),
                    value, strlen(value),
                    (time_t)0, (uint32_t)0);
  test_true(memcached_failed(rc));

  disconnected_server= memcached_server_get_last_disconnect(mine);
  test_true_got(disconnected_server, memcached_strerror(mine, rc));
  test_compare(in_port_t(9), memcached_server_port(disconnected_server));
  test_false(strncmp(memcached_server_name(disconnected_server),"localhost",9));

  memcached_quit(mine);
  memcached_free(mine);

  return TEST_SUCCESS;
}

test_return_t test_multiple_get_last_disconnect(memcached_st *)
{
  const char *server_string= "--server=localhost:8888 --server=localhost:8889 --server=localhost:8890 --server=localhost:8891 --server=localhost:8892";
  char buffer[BUFSIZ];

  test_compare(MEMCACHED_SUCCESS,
               libmemcached_check_configuration(server_string, strlen(server_string), buffer, sizeof(buffer)));

  memcached_st *memc= memcached(server_string, strlen(server_string));
  test_true(memc);

  // We will just use the error strings as our keys
  uint32_t counter= 100;
  while (--counter)
  {
    for (int x= int(MEMCACHED_SUCCESS); x < int(MEMCACHED_MAXIMUM_RETURN); ++x)
    {
      const char *msg=  memcached_strerror(memc, memcached_return_t(x));
      memcached_return_t ret= memcached_set(memc, msg, strlen(msg), NULL, 0, (time_t)0, (uint32_t)0);
      test_true_got((ret == MEMCACHED_CONNECTION_FAILURE or ret == MEMCACHED_SERVER_TEMPORARILY_DISABLED), memcached_last_error_message(memc));

      const memcached_instance_st * disconnected_server= memcached_server_get_last_disconnect(memc);
      test_true(disconnected_server);
      test_strcmp("localhost", memcached_server_name(disconnected_server));
      test_true(memcached_server_port(disconnected_server) >= 8888 and memcached_server_port(disconnected_server) <= 8892);

      if (random() % 2)
      {
        memcached_reset_last_disconnected_server(memc);
      }
    }
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t test_verbosity(memcached_st *memc)
{
  memcached_verbosity(memc, 3);

  return TEST_SUCCESS;
}


static memcached_return_t stat_printer(const memcached_instance_st * server,
                                       const char *key, size_t key_length,
                                       const char *value, size_t value_length,
                                       void *context)
{
  (void)server;
  (void)context;
  (void)key;
  (void)key_length;
  (void)value;
  (void)value_length;

  return MEMCACHED_SUCCESS;
}

test_return_t memcached_stat_execute_test(memcached_st *memc)
{
  memcached_return_t rc= memcached_stat_execute(memc, NULL, stat_printer, NULL);
  test_compare(MEMCACHED_SUCCESS, rc);

  test_compare(MEMCACHED_SUCCESS, 
               memcached_stat_execute(memc, "slabs", stat_printer, NULL));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_stat_execute(memc, "items", stat_printer, NULL));

  test_compare(MEMCACHED_SUCCESS, 
               memcached_stat_execute(memc, "sizes", stat_printer, NULL));

  return TEST_SUCCESS;
}

/*
 * This test ensures that the failure counter isn't incremented during
 * normal termination of the memcached instance.
 */
test_return_t wrong_failure_counter_test(memcached_st *original_memc)
{
  memcached_st* memc= create_single_instance_memcached(original_memc, NULL);

  /* Ensure that we are connected to the server by setting a value */
  memcached_return_t rc= memcached_set(memc,
                                       test_literal_param(__func__), // Key
                                       test_literal_param(__func__), // Value
                                       time_t(0), uint32_t(0));
  test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED);


  const memcached_instance_st * instance= memcached_server_instance_by_position(memc, 0);

  /* The test is to see that the memcached_quit doesn't increase the
   * the server failure conter, so let's ensure that it is zero
   * before sending quit
 */
  ((memcached_server_write_instance_st)instance)->server_failure_counter= 0;

  memcached_quit(memc);

  /* Verify that it memcached_quit didn't increment the failure counter
   * Please note that this isn't bullet proof, because an error could
   * occur...
 */
  test_zero(instance->server_failure_counter);

  memcached_free(memc);

  return TEST_SUCCESS;
}

/*
 * This tests ensures expected disconnections (for some behavior changes
 * for instance) do not wrongly increase failure counter
 */
test_return_t wrong_failure_counter_two_test(memcached_st *memc)
{
  /* Set value to force connection to the server */
  const char *key= "marmotte";
  const char *value= "milka";

  test_compare_hint(MEMCACHED_SUCCESS,
                    memcached_set(memc, key, strlen(key),
                                  value, strlen(value),
                                  (time_t)0, (uint32_t)0),
                    memcached_last_error_message(memc));


  /* put failure limit to 1 */
  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, true));

  /* Put a retry timeout to effectively activate failure_limit effect */
  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 1));

  /* change behavior that triggers memcached_quit()*/
  test_compare(MEMCACHED_SUCCESS,
               memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, true));


  /* Check if we still are connected */
  uint32_t flags;
  size_t string_length;
  memcached_return rc;
  char *string= memcached_get(memc, key, strlen(key),
                              &string_length, &flags, &rc);

  test_compare_got(MEMCACHED_SUCCESS, rc, memcached_strerror(NULL, rc));
  test_true(string);
  free(string);

  return TEST_SUCCESS;
}

test_return_t regression_996813_TEST(memcached_st *)
{
  memcached_st* memc= memcached_create(NULL);

  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, 1));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_POLL_TIMEOUT, 1));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, 300));
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 30));

  // We will never connect to these servers
  in_port_t base_port= 11211;
  for (size_t x= 0; x < 17; x++)
  {
    test_compare(MEMCACHED_SUCCESS, memcached_server_add(memc, "10.2.3.4", base_port +x));
  }
  test_compare(6U, memcached_generate_hash(memc, test_literal_param("SZ6hu0SHweFmpwpc0w2R")));
  test_compare(1U, memcached_generate_hash(memc, test_literal_param("SQCK9eiCf53YxHWnYA.o")));
  test_compare(9U, memcached_generate_hash(memc, test_literal_param("SUSDkGXuuZC9t9VhMwa.")));
  test_compare(0U, memcached_generate_hash(memc, test_literal_param("SnnqnJARfaCNT679iAF_")));

  memcached_free(memc);

  return TEST_SUCCESS;
}


/*
 * Test that ensures mget_execute does not end into recursive calls that finally fails
 */
test_return_t regression_bug_490486(memcached_st *original_memc)
{

#ifdef __APPLE__
  return TEST_SKIPPED; // My MAC can't handle this test
#endif

  test_skip(TEST_SUCCESS, pre_binary(original_memc));

  /*
   * I only want to hit _one_ server so I know the number of requests I'm
   * sending in the pipeline.
 */
  memcached_st *memc= create_single_instance_memcached(original_memc, "--BINARY-PROTOCOL --POLL-TIMEOUT=1000 --REMOVE-FAILED-SERVERS=1 --RETRY-TIMEOUT=3600");
  test_true(memc);

  keys_st keys(20480);

  /* First add all of the items.. */
  char blob[1024]= { 0 };
  for (size_t x= 0; x < keys.size(); ++x)
  {
    memcached_return rc= memcached_set(memc,
                                       keys.key_at(x), keys.length_at(x),
                                       blob, sizeof(blob), 0, 0);
    test_true(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED); // MEMCACHED_TIMEOUT <-- hash been observed on OSX
  }

  {

    /* Try to get all of them with a large multiget */
    size_t counter= 0;
    memcached_execute_function callbacks[]= { &callback_counter };
    memcached_return_t rc= memcached_mget_execute(memc,
                                                  keys.keys_ptr(), keys.lengths_ptr(), keys.size(),
                                                  callbacks, &counter, 1);
    test_compare(MEMCACHED_SUCCESS, rc);

    char* the_value= NULL;
    char the_key[MEMCACHED_MAX_KEY];
    size_t the_key_length;
    size_t the_value_length;
    uint32_t the_flags;

    do {
      the_value= memcached_fetch(memc, the_key, &the_key_length, &the_value_length, &the_flags, &rc);

      if ((the_value!= NULL) && (rc == MEMCACHED_SUCCESS))
      {
        ++counter;
        free(the_value);
      }

    } while ( (the_value!= NULL) && (rc == MEMCACHED_SUCCESS));


    test_compare(MEMCACHED_END, rc);

    /* Verify that we got all of the items */
    test_compare(counter, keys.size());
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t regression_1021819_TEST(memcached_st *original)
{
  memcached_st *memc= memcached_clone(NULL, original);
  test_true(memc);

  test_compare(memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SND_TIMEOUT, 2000000), MEMCACHED_SUCCESS);
  test_compare(memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RCV_TIMEOUT, 3000000), MEMCACHED_SUCCESS);

  memcached_return_t rc;

  memcached_get(memc,
                test_literal_param(__func__),
                NULL, NULL, &rc);

  test_compare(rc, MEMCACHED_NOTFOUND);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t regression_bug_583031(memcached_st *)
{
  memcached_st *memc= memcached_create(NULL);
  test_true(memc);
  test_compare(MEMCACHED_SUCCESS, memcached_server_add(memc, "10.2.251.4", 11211));

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, 3000);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 1000);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SND_TIMEOUT, 1000);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RCV_TIMEOUT, 1000);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_POLL_TIMEOUT, 1000);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, 3);

  memcached_return_t rc;
  size_t length;
  uint32_t flags;

  const char *value= memcached_get(memc, "dsf", 3, &length, &flags, &rc);
  test_false(value);
  test_zero(length);

  test_compare(MEMCACHED_TIMEOUT, memc);

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t regression_bug_581030(memcached_st *)
{
#ifndef DEBUG
  memcached_stat_st *local_stat= memcached_stat(NULL, NULL, NULL);
  test_false(local_stat);

  memcached_stat_free(NULL, NULL);
#endif

  return TEST_SUCCESS;
}

#define regression_bug_655423_COUNT 6000
test_return_t regression_bug_655423(memcached_st *memc)
{
  memcached_st *clone= memcached_clone(NULL, memc);
  memc= NULL; // Just to make sure it is not used
  test_true(clone);
  char payload[100];

#ifdef __APPLE__
  return TEST_SKIPPED;
#endif

  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(clone, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1));
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(clone, MEMCACHED_BEHAVIOR_SUPPORT_CAS, 1));
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(clone, MEMCACHED_BEHAVIOR_TCP_NODELAY, 1));
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(clone, MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH, 1));

  memset(payload, int('x'), sizeof(payload));

  keys_st keys(regression_bug_655423_COUNT);

  for (size_t x= 0; x < keys.size(); x++)
  {
    test_compare(MEMCACHED_SUCCESS, memcached_set(clone, 
                                                  keys.key_at(x),
                                                  keys.length_at(x),
                                                  payload, sizeof(payload), 0, 0));
  }

  for (size_t x= 0; x < keys.size(); x++)
  {
    size_t value_length;
    memcached_return_t rc;
    char *value= memcached_get(clone,
                               keys.key_at(x),
                               keys.length_at(x),
                               &value_length, NULL, &rc);

    if (rc == MEMCACHED_NOTFOUND)
    {
      test_false(value);
      test_zero(value_length);
      continue;
    }

    test_compare(MEMCACHED_SUCCESS, rc);
    test_true(value);
    test_compare(100LLU, value_length);
    free(value);
  }

  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(clone,
                              keys.keys_ptr(), keys.lengths_ptr(),
                              keys.size()));

  uint32_t count= 0;
  memcached_result_st *result= NULL;
  while ((result= memcached_fetch_result(clone, result, NULL)))
  {
    test_compare(size_t(100), memcached_result_length(result));
    count++;
  }

  test_true(count > 100); // If we don't get back atleast this, something is up

  memcached_free(clone);

  return TEST_SUCCESS;
}

/*
 * Test that ensures that buffered set to not trigger problems during io_flush
 */
#define regression_bug_490520_COUNT 200480
test_return_t regression_bug_490520(memcached_st *original_memc)
{
  memcached_st* memc= create_single_instance_memcached(original_memc, NULL);

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK,1);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS,1);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_POLL_TIMEOUT, 1000);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT,1);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 3600);

  /* First add all of the items.. */
  char blob[3333] = {0};
  for (uint32_t x= 0; x < regression_bug_490520_COUNT; ++x)
  {
    char key[251];
    int key_length= snprintf(key, sizeof(key), "0200%u", x);

    memcached_return rc= memcached_set(memc, key, key_length, blob, sizeof(blob), 0, 0);
    test_true_got(rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED, memcached_last_error_message(memc));
  }

  memcached_free(memc);

  return TEST_SUCCESS;
}

test_return_t regression_bug_1251482(memcached_st*)
{
  test::Memc memc("--server=localhost:5");

  memcached_behavior_set(&memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 0);

  for (size_t x= 4; x; --x)
  {
    size_t value_length;
    memcached_return_t rc;
    char *value= memcached_get(&memc,
                               test_literal_param(__func__),
                               &value_length, NULL, &rc);

    test_false(value);
    test_compare(0LLU, value_length);
    test_compare(MEMCACHED_CONNECTION_FAILURE, rc);
  }

  return TEST_SUCCESS;
}

test_return_t regression_1009493_TEST(memcached_st*)
{
  memcached_st* memc= memcached_create(NULL);
  test_true(memc);
  test_compare(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_KETAMA, true));

  memcached_st* clone= memcached_clone(NULL, memc);
  test_true(clone);

  test_compare(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED),
               memcached_behavior_get(clone, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED));

  memcached_free(memc);
  memcached_free(clone);

  return TEST_SUCCESS;
}

test_return_t regression_994772_TEST(memcached_st* memc)
{
  test_skip(MEMCACHED_SUCCESS, memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1));

  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc,
                             test_literal_param(__func__), // Key
                             test_literal_param(__func__), // Value
                             time_t(0), uint32_t(0)));

  const char *keys[] = { __func__ };
  size_t key_length[]= { strlen(__func__) };
  test_compare(MEMCACHED_SUCCESS,
               memcached_mget(memc, keys, key_length, 1));

  memcached_return_t rc;
  memcached_result_st *results= memcached_fetch_result(memc, NULL, &rc);
  test_true(results);
  test_compare(MEMCACHED_SUCCESS, rc);

  test_strcmp(__func__, memcached_result_value(results));
  uint64_t cas_value= memcached_result_cas(results);
  test_true(cas_value);

  char* take_value= memcached_result_take_value(results);
  test_strcmp(__func__, take_value);
  free(take_value);

  memcached_result_free(results);

  // Bad cas value, sanity check 
  test_true(cas_value != 9999);
  test_compare(MEMCACHED_END,
               memcached_cas(memc,
                             test_literal_param(__func__), // Key
                             test_literal_param(__FILE__), // Value
                             time_t(0), uint32_t(0), 9999));

  test_compare(MEMCACHED_SUCCESS, memcached_set(memc,
                                                "different", strlen("different"), // Key
                                                test_literal_param(__FILE__), // Value
                                                time_t(0), uint32_t(0)));

  return TEST_SUCCESS;
}

test_return_t regression_bug_854604(memcached_st *)
{
  char buffer[1024];

  test_compare(MEMCACHED_INVALID_ARGUMENTS, libmemcached_check_configuration(0, 0, buffer, 0));

  test_compare(MEMCACHED_PARSE_ERROR, libmemcached_check_configuration(test_literal_param("syntax error"), buffer, 0));

  test_compare(MEMCACHED_PARSE_ERROR, libmemcached_check_configuration(test_literal_param("syntax error"), buffer, 1));
  test_compare(buffer[0], 0);

  test_compare(MEMCACHED_PARSE_ERROR, libmemcached_check_configuration(test_literal_param("syntax error"), buffer, 10));
  test_true(strlen(buffer));

  test_compare(MEMCACHED_PARSE_ERROR, libmemcached_check_configuration(test_literal_param("syntax error"), buffer, sizeof(buffer)));
  test_true(strlen(buffer));

  return TEST_SUCCESS;
}

static void die_message(memcached_st* mc, memcached_return error, const char* what, uint32_t it)
{
  fprintf(stderr, "Iteration #%u: ", it);

  if (error == MEMCACHED_ERRNO)
  {
    fprintf(stderr, "system error %d from %s: %s\n",
            errno, what, strerror(errno));
  }
  else
  {
    fprintf(stderr, "error %d from %s: %s\n", error, what,
            memcached_strerror(mc, error));
  }
}

#define TEST_CONSTANT_CREATION 200

test_return_t regression_bug_(memcached_st *memc)
{
  const char *remote_server;
  (void)memc;

  if (! (remote_server= getenv("LIBMEMCACHED_REMOTE_SERVER")))
  {
    return TEST_SKIPPED;
  }

  for (uint32_t x= 0; x < TEST_CONSTANT_CREATION; x++)
  {
    memcached_st* mc= memcached_create(NULL);
    memcached_return rc;

    rc= memcached_behavior_set(mc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);
    if (rc != MEMCACHED_SUCCESS)
    {
      die_message(mc, rc, "memcached_behavior_set", x);
    }

    rc= memcached_behavior_set(mc, MEMCACHED_BEHAVIOR_CACHE_LOOKUPS, 1);
    if (rc != MEMCACHED_SUCCESS)
    {
      die_message(mc, rc, "memcached_behavior_set", x);
    }

    rc= memcached_server_add(mc, remote_server, 0);
    if (rc != MEMCACHED_SUCCESS)
    {
      die_message(mc, rc, "memcached_server_add", x);
    }

    const char *set_key= "akey";
    const size_t set_key_len= strlen(set_key);
    const char *set_value= "a value";
    const size_t set_value_len= strlen(set_value);

    if (rc == MEMCACHED_SUCCESS)
    {
      if (x > 0)
      {
        size_t get_value_len;
        char *get_value;
        uint32_t get_value_flags;

        get_value= memcached_get(mc, set_key, set_key_len, &get_value_len,
                                 &get_value_flags, &rc);
        if (rc != MEMCACHED_SUCCESS)
        {
          die_message(mc, rc, "memcached_get", x);
        }
        else
        {

          if (x != 0 &&
              (get_value_len != set_value_len
               || 0!=strncmp(get_value, set_value, get_value_len)))
          {
            fprintf(stderr, "Values don't match?\n");
            rc= MEMCACHED_FAILURE;
          }
          free(get_value);
        }
      }

      rc= memcached_set(mc,
                        set_key, set_key_len,
                        set_value, set_value_len,
                        0, /* time */
                        0  /* flags */
                       );
      if (rc != MEMCACHED_SUCCESS)
      {
        die_message(mc, rc, "memcached_set", x);
      }
    }

    memcached_quit(mc);
    memcached_free(mc);

    if (rc != MEMCACHED_SUCCESS)
    {
      break;
    }
  }

  return TEST_SUCCESS;
}

test_return_t kill_HUP_TEST(memcached_st *original_memc)
{
  memcached_st *memc= create_single_instance_memcached(original_memc, 0);
  test_true(memc);

  const memcached_instance_st * instance= memcached_server_instance_by_position(memc, 0);

  pid_t pid;
  test_true((pid= libmemcached_util_getpid(memcached_server_name(instance),
                                           memcached_server_port(instance), NULL)) > -1);


  test_compare(MEMCACHED_SUCCESS,
               memcached_set(memc, 
                             test_literal_param(__func__), // Keys
                             test_literal_param(__func__), // Values
                             0, 0));
  test_true_got(kill(pid, SIGHUP) == 0, strerror(errno));

  memcached_return_t ret= memcached_set(memc, 
                                        test_literal_param(__func__), // Keys
                                        test_literal_param(__func__), // Values
                                        0, 0);
  test_compare(ret, memc);
  test_compare(MEMCACHED_CONNECTION_FAILURE, memc);

  memcached_free(memc);

  return TEST_SUCCESS;
}
