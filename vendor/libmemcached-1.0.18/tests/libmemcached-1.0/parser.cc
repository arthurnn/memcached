/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached Client and Server 
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

#include <vector>
#include <string>
#include <cerrno>

#include <libmemcached-1.0/memcached.h>
#include <libmemcachedutil-1.0/util.h>

#include <tests/libmemcached-1.0/parser.h>
#include <tests/print.h>
#include "libmemcached/instance.hpp"

enum scanner_type_t
{
  NIL,
  UNSIGNED,
  SIGNED,
  ARRAY
};


struct scanner_string_st {
  const char *c_str;
  size_t size;
};

static inline scanner_string_st scanner_string(const char *arg, size_t arg_size)
{
  scanner_string_st local= { arg, arg_size };
  return local;
}

#define make_scanner_string(X) scanner_string((X), static_cast<size_t>(sizeof(X) - 1))

static struct scanner_string_st scanner_string_null= { 0, 0};

struct scanner_variable_t {
  enum scanner_type_t type;
  struct scanner_string_st option;
  struct scanner_string_st result;
  test_return_t (*check_func)(memcached_st *memc, const scanner_string_st &hostname);
};

// Check and make sure the first host is what we expect it to be
static test_return_t __check_host(memcached_st *memc, const scanner_string_st &hostname)
{
  const memcached_instance_st * instance=
    memcached_server_instance_by_position(memc, 0);

  test_true(instance);

  const char *first_hostname = memcached_server_name(instance);
  test_true(first_hostname);
  test_strcmp(first_hostname, hostname.c_str);

  return TEST_SUCCESS;
}

// Check and make sure the prefix_key is what we expect it to be
static test_return_t __check_namespace(memcached_st *, const scanner_string_st &)
{
#if 0
  const char *_namespace = memcached_get_namespace(memc);
  test_true(_namespace);
  test_strcmp(_namespace, arg.c_str);
#endif

  return TEST_SUCCESS;
}

static test_return_t __check_IO_MSG_WATERMARK(memcached_st *memc, const scanner_string_st &value)
{
  uint64_t value_number;

  value_number= atoll(value.c_str);

  test_true(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK) == value_number);
  return TEST_SUCCESS;
}

static test_return_t __check_REMOVE_FAILED_SERVERS(memcached_st *memc, const scanner_string_st &)
{
  test_true(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS));
  return TEST_SUCCESS;
}

static test_return_t __check_NOREPLY(memcached_st *memc, const scanner_string_st &)
{
  test_true(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_NOREPLY));
  return TEST_SUCCESS;
}

static test_return_t __check_VERIFY_KEY(memcached_st *memc, const scanner_string_st &)
{
  test_true(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_VERIFY_KEY));
  return TEST_SUCCESS;
}

static test_return_t __check_distribution_RANDOM(memcached_st *memc, const scanner_string_st &)
{
  test_true(memcached_behavior_get(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION) == MEMCACHED_DISTRIBUTION_RANDOM);
  return TEST_SUCCESS;
}

scanner_variable_t test_server_strings[]= {
  { ARRAY, make_scanner_string("--server=localhost"), make_scanner_string("localhost"), __check_host },
  { ARRAY, make_scanner_string("--server=10.0.2.1"), make_scanner_string("10.0.2.1"), __check_host },
  { ARRAY, make_scanner_string("--server=example.com"), make_scanner_string("example.com"), __check_host },
  { ARRAY, make_scanner_string("--server=localhost:30"), make_scanner_string("localhost"), __check_host },
  { ARRAY, make_scanner_string("--server=10.0.2.1:20"), make_scanner_string("10.0.2.1"), __check_host },
  { ARRAY, make_scanner_string("--server=example.com:1024"), make_scanner_string("example.com"), __check_host },
  { NIL, scanner_string_null, scanner_string_null, NULL }
};

scanner_variable_t test_server_strings_with_weights[]= {
  { ARRAY, make_scanner_string("--server=10.0.2.1:30/?40"), make_scanner_string("10.0.2.1"), __check_host },
  { ARRAY, make_scanner_string("--server=example.com:1024/?30"), make_scanner_string("example.com"), __check_host },
  { ARRAY, make_scanner_string("--server=10.0.2.1/?20"), make_scanner_string("10.0.2.1"), __check_host },
  { ARRAY, make_scanner_string("--server=example.com/?10"), make_scanner_string("example.com"), __check_host },
  { NIL, scanner_string_null, scanner_string_null, NULL }
};

scanner_variable_t bad_test_strings[]= {
  { ARRAY, make_scanner_string("-servers=localhost:11221,localhost:11222,localhost:11223,localhost:11224,localhost:11225"), scanner_string_null, NULL },
  { ARRAY, make_scanner_string("-- servers=a.example.com:81,localhost:82,b.example.com"), scanner_string_null, NULL },
  { ARRAY, make_scanner_string("--servers=localhost:+80"), scanner_string_null, NULL},
  { ARRAY, make_scanner_string("--servers=localhost.com."), scanner_string_null, NULL},
  { ARRAY, make_scanner_string("--server=localhost.com."), scanner_string_null, NULL},
  { ARRAY, make_scanner_string("--server=localhost.com.:80"), scanner_string_null, NULL},
  { NIL, scanner_string_null, scanner_string_null, NULL}
};

scanner_variable_t test_number_options[]= {
  { ARRAY,  make_scanner_string("--CONNECT-TIMEOUT=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--IO-BYTES-WATERMARK=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--IO-KEY-PREFETCH=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--IO-MSG-WATERMARK=456"), make_scanner_string("456"), __check_IO_MSG_WATERMARK },
  { ARRAY,  make_scanner_string("--NUMBER-OF-REPLICAS=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--POLL-TIMEOUT=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--RCV-TIMEOUT=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--REMOVE-FAILED-SERVERS=3"), scanner_string_null, __check_REMOVE_FAILED_SERVERS },
  { ARRAY,  make_scanner_string("--RETRY-TIMEOUT=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--SND-TIMEOUT=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--SOCKET-RECV-SIZE=456"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--SOCKET-SEND-SIZE=456"), scanner_string_null, NULL },
  { NIL, scanner_string_null, scanner_string_null, NULL}
};

scanner_variable_t test_boolean_options[]= {
  { ARRAY,  make_scanner_string("--FETCH-VERSION"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--BINARY-PROTOCOL"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--BUFFER-REQUESTS"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--HASH-WITH-NAMESPACE"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--NOREPLY"), scanner_string_null, __check_NOREPLY },
  { ARRAY,  make_scanner_string("--RANDOMIZE-REPLICA-READ"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--SORT-HOSTS"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--SUPPORT-CAS"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--TCP-NODELAY"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--TCP-KEEPALIVE"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--TCP-KEEPIDLE"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--USE-UDP"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--VERIFY-KEY"), scanner_string_null, __check_VERIFY_KEY },
  { NIL, scanner_string_null, scanner_string_null, NULL}
};

scanner_variable_t namespace_strings[]= {
  { ARRAY, make_scanner_string("--NAMESPACE=foo"), make_scanner_string("foo"), __check_namespace },
  { ARRAY, make_scanner_string("--NAMESPACE=\"foo\""), make_scanner_string("foo"), __check_namespace },
  { ARRAY, make_scanner_string("--NAMESPACE=\"This_is_a_very_long_key\""), make_scanner_string("This_is_a_very_long_key"), __check_namespace },
  { NIL, scanner_string_null, scanner_string_null, NULL}
};

scanner_variable_t distribution_strings[]= {
  { ARRAY,  make_scanner_string("--DISTRIBUTION=consistent"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--DISTRIBUTION=consistent,CRC"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--DISTRIBUTION=consistent,MD5"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--DISTRIBUTION=random"), scanner_string_null, __check_distribution_RANDOM },
  { ARRAY,  make_scanner_string("--DISTRIBUTION=modula"), scanner_string_null, NULL },
  { NIL, scanner_string_null, scanner_string_null, NULL}
};

scanner_variable_t hash_strings[]= {
  { ARRAY,  make_scanner_string("--HASH=CRC"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--HASH=FNV1A_32"), scanner_string_null, NULL },
  { ARRAY,  make_scanner_string("--HASH=FNV1_32"), scanner_string_null, NULL },
#if 0
  { ARRAY,  make_scanner_string("--HASH=JENKINS"), scanner_string_null, NULL },
#endif
  { ARRAY,  make_scanner_string("--HASH=MD5"), scanner_string_null, NULL },
  { NIL, scanner_string_null, scanner_string_null, NULL}
};


static test_return_t _test_option(scanner_variable_t *scanner, bool test_true_opt= true)
{
  for (scanner_variable_t *ptr= scanner; ptr->type != NIL; ptr++)
  {
    memcached_st *memc= memcached(ptr->option.c_str, ptr->option.size);

    // The case that it should have parsed, but it didn't. We will inspect for
    // an error with libmemcached_check_configuration()
    if (memc == NULL and test_true_opt)
    {
      char buffer[2048];
      bool success= libmemcached_check_configuration(ptr->option.c_str, ptr->option.size, buffer, sizeof(buffer));

      std::string temp(buffer);
      temp+= " with option string:";
      temp+= ptr->option.c_str;
      test_true_got(success, temp.c_str());
      Error << "Failed for " << temp;

      return TEST_FAILURE; // The line above should fail since memc should be null
    }

    if (test_true_opt)
    {
      if (ptr->check_func)
      {
        test_return_t test_rc= (*ptr->check_func)(memc, ptr->result);
        if (test_rc != TEST_SUCCESS)
        {
          memcached_free(memc);
          return test_rc;
        }
      }

      memcached_free(memc);
    }
    else
    {
      test_false_with(memc, ptr->option.c_str);
    }
  }

  return TEST_SUCCESS;
}

test_return_t server_test(memcached_st *)
{
  return _test_option(test_server_strings);
}

test_return_t server_with_weight_test(memcached_st *)
{
  return _test_option(test_server_strings_with_weights);
}

test_return_t servers_bad_test(memcached_st *)
{
  test_return_t rc;
  if ((rc= _test_option(bad_test_strings, false)) != TEST_SUCCESS)
  {
    return rc;
  }

  return TEST_SUCCESS;
}

test_return_t parser_number_options_test(memcached_st*)
{
  return _test_option(test_number_options);
}

test_return_t parser_boolean_options_test(memcached_st*)
{
  return _test_option(test_boolean_options);
}

test_return_t behavior_parser_test(memcached_st*)
{
  return TEST_SUCCESS;
}

test_return_t parser_hash_test(memcached_st*)
{
  return _test_option(hash_strings);
}

test_return_t parser_distribution_test(memcached_st*)
{
  return _test_option(distribution_strings);
}

test_return_t parser_key_prefix_test(memcached_st*)
{
  return _test_option(distribution_strings);
}

test_return_t test_namespace_keyword(memcached_st*)
{
  return _test_option(namespace_strings);
}

#define SUPPORT_EXAMPLE_CNF "support/example.cnf"

test_return_t memcached_create_with_options_with_filename(memcached_st*)
{
  test_skip(0, access(SUPPORT_EXAMPLE_CNF, R_OK));

  memcached_st *memc_ptr= memcached(test_literal_param("--CONFIGURE-FILE=\"support/example.cnf\""));
  test_true_got(memc_ptr, "memcached() failed");
  memcached_free(memc_ptr);

  return TEST_SUCCESS;
}

test_return_t libmemcached_check_configuration_with_filename_test(memcached_st*)
{
  test_skip(0, access(SUPPORT_EXAMPLE_CNF, R_OK));

  char buffer[BUFSIZ];

  test_compare_hint(MEMCACHED_SUCCESS,
                    libmemcached_check_configuration(test_literal_param("--CONFIGURE-FILE=\"support/example.cnf\""), buffer, sizeof(buffer)),
                    buffer);

  test_compare_hint(MEMCACHED_SUCCESS,
                    libmemcached_check_configuration(test_literal_param("--CONFIGURE-FILE=support/example.cnf"), buffer, sizeof(buffer)),
                    buffer);

  test_compare_hint(MEMCACHED_ERRNO,
                    libmemcached_check_configuration(test_literal_param("--CONFIGURE-FILE=\"bad-path/example.cnf\""), buffer, sizeof(buffer)),
                    buffer) ;

  return TEST_SUCCESS;
}

test_return_t libmemcached_check_configuration_test(memcached_st*)
{
  char buffer[BUFSIZ];
  test_compare(MEMCACHED_SUCCESS,
               libmemcached_check_configuration(test_literal_param("--server=localhost"), buffer, sizeof(buffer)));

  test_compare_hint(MEMCACHED_PARSE_ERROR,
                    libmemcached_check_configuration(test_literal_param("--dude=localhost"), buffer, sizeof(buffer)),
                    buffer);

  return TEST_SUCCESS;
}

test_return_t memcached_create_with_options_test(memcached_st*)
{
  {
    memcached_st *memc_ptr;
    memc_ptr= memcached(test_literal_param("--server=localhost"));
    test_true_got(memc_ptr, memcached_last_error_message(memc_ptr));
    memcached_free(memc_ptr);
  }

  {
    memcached_st *memc_ptr= memcached(test_literal_param("--dude=localhost"));
    test_false_with(memc_ptr, memcached_last_error_message(memc_ptr));
  }

  return TEST_SUCCESS;
}

test_return_t test_include_keyword(memcached_st*)
{
  test_skip(0, access(SUPPORT_EXAMPLE_CNF, R_OK));

  char buffer[BUFSIZ];
  test_compare(MEMCACHED_SUCCESS, 
               libmemcached_check_configuration(test_literal_param("INCLUDE \"support/example.cnf\""), buffer, sizeof(buffer)));

  return TEST_SUCCESS;
}

test_return_t test_end_keyword(memcached_st*)
{
  char buffer[BUFSIZ];
  test_compare(MEMCACHED_SUCCESS, 
               libmemcached_check_configuration(test_literal_param("--server=localhost END bad keywords"), buffer, sizeof(buffer)));

  return TEST_SUCCESS;
}

test_return_t test_reset_keyword(memcached_st*)
{
  char buffer[BUFSIZ];
  test_compare(MEMCACHED_SUCCESS,
               libmemcached_check_configuration(test_literal_param("--server=localhost reset --server=bad.com"), buffer, sizeof(buffer)));

  return TEST_SUCCESS;
}

test_return_t test_error_keyword(memcached_st*)
{
  char buffer[BUFSIZ];
  memcached_return_t rc;
  rc= libmemcached_check_configuration(test_literal_param("--server=localhost ERROR --server=bad.com"), buffer, sizeof(buffer));
  test_true_got(rc != MEMCACHED_SUCCESS, buffer);

  return TEST_SUCCESS;
}

#define RANDOM_STRINGS 1000
test_return_t random_statement_build_test(memcached_st*)
{
  std::vector<scanner_string_st *> option_list;

  for (scanner_variable_t *ptr= test_server_strings; ptr->type != NIL; ptr++)
    option_list.push_back(&ptr->option);

  for (scanner_variable_t *ptr= test_number_options; ptr->type != NIL; ptr++)
    option_list.push_back(&ptr->option);

  for (scanner_variable_t *ptr= test_boolean_options; ptr->type != NIL; ptr++)
    option_list.push_back(&ptr->option);

  for (scanner_variable_t *ptr= namespace_strings; ptr->type != NIL; ptr++)
    option_list.push_back(&ptr->option);

  for (scanner_variable_t *ptr= distribution_strings; ptr->type != NIL; ptr++)
    option_list.push_back(&ptr->option);

  for (scanner_variable_t *ptr= hash_strings; ptr->type != NIL; ptr++)
    option_list.push_back(&ptr->option);

  std::vector<bool> used_list;
  used_list.resize(option_list.size());

  struct used_options_st {
    bool has_hash;
    bool has_namespace;
    bool has_distribution;
    bool has_buffer_requests;
    bool has_udp;
    bool has_binary;
    bool has_verify_key;

    used_options_st() :
      has_hash(false),
      has_namespace(false),
      has_distribution(false),
      has_buffer_requests(false),
      has_udp(false),
      has_binary(false),
      has_verify_key(false)
    {
    }
  } used_options;

  for (uint32_t x= 0; x < RANDOM_STRINGS; x++)
  {
    std::string random_options;

    uint32_t number_of= random() % uint32_t(option_list.size());
    for (uint32_t options= 0; options < number_of; options++)
    {
      size_t option_list_position= random() % option_list.size();

      if (used_list[option_list_position])
      {
        continue;
      }
      used_list[option_list_position]= true;

      std::string random_string= option_list[option_list_position]->c_str;

      if (random_string.compare(0, test_literal_compare_param("--HASH")) == 0)
      {
        if (used_options.has_hash)
        {
          continue;
        }

        if (used_options.has_distribution)
        {
          continue;
        }
        used_options.has_hash= true;
      }

      if (random_string.compare(0, test_literal_compare_param("--NAMESPACE")) == 0)
      {
        if (used_options.has_namespace)
        {
          continue;
        }
        used_options.has_namespace= true;
      }

      if (random_string.compare(0, test_literal_compare_param("--USE-UDP")) == 0)
      {
        if (used_options.has_udp)
        {
          continue;
        }
        used_options.has_udp= true;

        if (used_options.has_buffer_requests)
        {
          continue;
        }
      }

      if (random_string.compare(0, test_literal_compare_param("--BUFFER-REQUESTS")) == 0)
      {
        if (used_options.has_buffer_requests)
        {
          continue;
        }
        used_options.has_buffer_requests= true;

        if (used_options.has_udp)
        {
          continue;
        }
      }

      if (random_string.compare(0, test_literal_compare_param("--BINARY-PROTOCOL")) == 0)
      {
        if (used_options.has_binary)
        {
          continue;
        }
        used_options.has_binary= true;

        if (used_options.has_verify_key)
        {
          continue;
        }
      }

      if (random_string.compare(0, test_literal_compare_param("--VERIFY-KEY")) == 0)
      {
        if (used_options.has_verify_key)
        {
          continue;
        }
        used_options.has_verify_key= true;

        if (used_options.has_binary)
        {
          continue;
        }
      }

      if (random_string.compare(0, test_literal_compare_param("--DISTRIBUTION")) == 0)
      {
        if (used_options.has_distribution)
        {
          continue;
        }

        if (used_options.has_hash)
        {
          continue;
        }
        used_options.has_distribution= true;
      }

      random_options+= random_string;
      random_options+= " ";
    }

    if (random_options.size() <= 1)
    {
      continue;
    }

    random_options.resize(random_options.size() -1);

    char buffer[BUFSIZ];
    memcached_return_t rc= libmemcached_check_configuration(random_options.c_str(), random_options.size(), buffer, sizeof(buffer));
    if (memcached_failed(rc))
    {
      Error << "libmemcached_check_configuration(" << random_options << ") : " << buffer;
      return TEST_FAILURE;
    }
  }

  return TEST_SUCCESS;
}

static memcached_return_t dump_server_information(const memcached_st *,
                                                  const memcached_instance_st * instance,
                                                  void *)
{
  if (strcmp(memcached_server_name(instance), "localhost")) 
  {
    fatal_assert(not memcached_server_name(instance));
    return MEMCACHED_FAILURE;
  }

  if (memcached_server_port(instance) < 8888 or memcached_server_port(instance) > 8892)
  {
    fatal_assert(not memcached_server_port(instance));
    return MEMCACHED_FAILURE;
  }

  if (instance->weight > 5 or instance->weight < 2)
  {
    fatal_assert(not instance->weight);
    return MEMCACHED_FAILURE;
  }

  return MEMCACHED_SUCCESS;
}

test_return_t test_hostname_port_weight(memcached_st *)
{
  const char *server_string= "--server=localhost:8888/?2 --server=localhost:8889/?3 --server=localhost:8890/?4 --server=localhost:8891/?5 --server=localhost:8892/?3";
  char buffer[BUFSIZ];

  test_compare_got(MEMCACHED_SUCCESS,
                   libmemcached_check_configuration(server_string, strlen(server_string), buffer, sizeof(buffer)), buffer);

  memcached_st *memc= memcached(server_string, strlen(server_string));
  test_true(memc);

  memcached_server_fn callbacks[]= { dump_server_information };
  test_true(memcached_success(memcached_server_cursor(memc, callbacks, NULL, 1)));

  memcached_free(memc);

  return TEST_SUCCESS;
}

struct socket_weight_t {
  const char *socket;
  size_t weight;
  const char* type;
};

static memcached_return_t dump_socket_information(const memcached_st *,
                                                  const memcached_instance_st * instance,
                                                  void *context)
{
  socket_weight_t *check= (socket_weight_t *)context;

  if (strcmp(memcached_server_type(instance), check->type) == 0)
  {
    if (strcmp(memcached_server_name(instance), check->socket) == 0)
    {
      if (instance->weight == check->weight)
      {
        return MEMCACHED_SUCCESS;
      }
      else
      {
        Error << instance->weight << " != " << check->weight;
      }
    }
    else
    {
      Error << "'" << memcached_server_name(instance) << "'" << " != " << "'" << check->socket << "'";
    }
  }
  else
  {
    Error << "'" << memcached_server_type(instance) << "'" << " != " << "'" << check->type << "'";
  }

  return MEMCACHED_FAILURE;
}

test_return_t test_parse_socket(memcached_st *)
{
  char buffer[BUFSIZ];

  memcached_server_fn callbacks[]= { dump_socket_information };
  {
    test_compare_got(MEMCACHED_SUCCESS,
                     libmemcached_check_configuration(test_literal_param("--socket=\"/tmp/foo\""), buffer, sizeof(buffer)),
                     buffer);

    memcached_st *memc= memcached(test_literal_param("--socket=\"/tmp/foo\""));
    test_true(memc);
    socket_weight_t check= { "/tmp/foo", 1, "SOCKET"};
    test_compare(MEMCACHED_SUCCESS,
                 memcached_server_cursor(memc, callbacks, &check, 1));
    memcached_free(memc);
  }

  {
    test_compare_got(MEMCACHED_SUCCESS,
                     libmemcached_check_configuration(test_literal_param("--socket=\"/tmp/foo\"/?23"), buffer, sizeof(buffer)),
                     buffer);

    memcached_st *memc= memcached(test_literal_param("--socket=\"/tmp/foo\"/?23"));
    test_true(memc);
    socket_weight_t check= { "/tmp/foo", 23, "SOCKET"};
    test_compare(MEMCACHED_SUCCESS,
                 memcached_server_cursor(memc, callbacks, &check, 1));
    memcached_free(memc);
  }

  return TEST_SUCCESS;
}

/*
  By setting the timeout value to zero, we force poll() to return immediatly.
*/
test_return_t regression_bug_71231153_connect(memcached_st *)
{
  if (libmemcached_util_ping("10.0.2.252", 0, NULL)) // If for whatever reason someone has a host at this address, skip
    return TEST_SKIPPED;

  { // Test the connect-timeout, on a bad host we should get MEMCACHED_CONNECTION_FAILURE
    memcached_st *memc= memcached(test_literal_param("--SERVER=10.0.2.252 --CONNECT-TIMEOUT=0"));
    test_true(memc);
    test_zero(memc->connect_timeout);
    test_compare(MEMCACHED_DEFAULT_TIMEOUT, memc->poll_timeout);

    memcached_return_t rc;
    size_t value_len;
    char *value= memcached_get(memc, test_literal_param("test"), &value_len, NULL, &rc);
    test_false(value);
    test_zero(value_len);
    test_compare_got(MEMCACHED_TIMEOUT, rc, memcached_last_error_message(memc));

    memcached_free(memc);
  }

  return TEST_SUCCESS;
}

test_return_t regression_bug_71231153_poll(memcached_st *)
{
  if (libmemcached_util_ping("10.0.2.252", 0, NULL)) // If for whatever reason someone has a host at this address, skip
  {
    return TEST_SKIPPED;
  }

  { // Test the poll timeout, on a bad host we should get MEMCACHED_CONNECTION_FAILURE
    memcached_st *memc= memcached(test_literal_param("--SERVER=10.0.2.252 --POLL-TIMEOUT=0"));
    test_true(memc);
    test_compare(MEMCACHED_DEFAULT_CONNECT_TIMEOUT, memc->connect_timeout);
    test_zero(memc->poll_timeout);

    memcached_return_t rc;
    size_t value_len;
    char *value= memcached_get(memc, test_literal_param("test"), &value_len, NULL, &rc);
    test_false(value);
    test_zero(value_len);
#ifdef __APPLE__
    test_compare_got(MEMCACHED_CONNECTION_FAILURE, rc, memcached_last_error_message(memc));
#else
    test_compare_got(MEMCACHED_TIMEOUT, rc, memcached_last_error_message(memc));
#endif

    memcached_free(memc);
  }

  return TEST_SUCCESS;
}
