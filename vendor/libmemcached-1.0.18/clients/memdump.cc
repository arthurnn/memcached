/* LibMemcached
 * Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
 * Copyright (C) 2006-2009 Brian Aker
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 *
 * Summary:
 *
 */

#include "mem_config.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>

#include <libmemcached-1.0/memcached.h>

#include "client_options.h"
#include "utilities.h"

#define PROGRAM_NAME "memdump"
#define PROGRAM_DESCRIPTION "Dump all values from one or many servers."

/* Prototypes */
static void options_parse(int argc, char *argv[]);

static bool opt_binary=0;
static int opt_verbose= 0;
static char *opt_servers= NULL;
static char *opt_hash= NULL;
static char *opt_username;
static char *opt_passwd;

/* Print the keys and counter how many were found */
static memcached_return_t key_printer(const memcached_st *,
                                      const char *key, size_t key_length,
                                      void *)
{
  std::cout.write(key, key_length);
  std::cout << std::endl;

  return MEMCACHED_SUCCESS;
}

int main(int argc, char *argv[])
{
  memcached_dump_fn callbacks[1];

  callbacks[0]= &key_printer;

  options_parse(argc, argv);

  if (opt_servers == NULL)
  {
    char *temp;

    if ((temp= getenv("MEMCACHED_SERVERS")))
    {
      opt_servers= strdup(temp);
    }
    else if (argc >= 1 and argv[--argc])
    {
      opt_servers= strdup(argv[argc]);
    }

    if (opt_servers == NULL)
    {
      std::cerr << "No Servers provided" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  memcached_server_st* servers= memcached_servers_parse(opt_servers);
  if (servers == NULL or memcached_server_list_count(servers) == 0)
  {
    std::cerr << "Invalid server list provided:" << opt_servers << std::endl;
    return EXIT_FAILURE;
  }

  memcached_st *memc= memcached_create(NULL);
  if (memc == NULL)
  {
    std::cerr << "Could not allocate a memcached_st structure.\n" << std::endl;
    return EXIT_FAILURE;
  }
  process_hash_option(memc, opt_hash);

  memcached_server_push(memc, servers);
  memcached_server_list_free(servers);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL,
                         (uint64_t)opt_binary);

  if (opt_username and LIBMEMCACHED_WITH_SASL_SUPPORT == 0)
  {
    memcached_free(memc);
    std::cerr << "--username was supplied, but binary was not built with SASL support." << std::endl;
    return EXIT_FAILURE;
  }

  if (opt_username)
  {
    memcached_return_t ret;
    if (memcached_failed(ret= memcached_set_sasl_auth_data(memc, opt_username, opt_passwd)))
    {
      std::cerr << memcached_last_error_message(memc) << std::endl;
      memcached_free(memc);
      return EXIT_FAILURE;
    }
  }

  memcached_return_t rc= memcached_dump(memc, callbacks, NULL, 1);

  int exit_code= EXIT_SUCCESS;
  if (memcached_failed(rc))
  {
    if (opt_verbose)
    {
      std::cerr << "Failed to dump keys: " << memcached_last_error_message(memc) << std::endl;
    }
    exit_code= EXIT_FAILURE;
  }

  memcached_free(memc);

  if (opt_servers)
  {
    free(opt_servers);
  }
  if (opt_hash)
  {
    free(opt_hash);
  }

  return exit_code;
}

static void options_parse(int argc, char *argv[])
{
  static struct option long_options[]=
    {
      {(OPTIONSTRING)"version", no_argument, NULL, OPT_VERSION},
      {(OPTIONSTRING)"help", no_argument, NULL, OPT_HELP},
      {(OPTIONSTRING)"quiet", no_argument, NULL, OPT_QUIET},
      {(OPTIONSTRING)"verbose", no_argument, &opt_verbose, OPT_VERBOSE},
      {(OPTIONSTRING)"debug", no_argument, &opt_verbose, OPT_DEBUG},
      {(OPTIONSTRING)"servers", required_argument, NULL, OPT_SERVERS},
      {(OPTIONSTRING)"hash", required_argument, NULL, OPT_HASH},
      {(OPTIONSTRING)"binary", no_argument, NULL, OPT_BINARY},
      {(OPTIONSTRING)"username", required_argument, NULL, OPT_USERNAME},
      {(OPTIONSTRING)"password", required_argument, NULL, OPT_PASSWD},
      {0, 0, 0, 0}
    };

  int option_index= 0;
  bool opt_version= false;
  bool opt_help= false;
  while (1)
  {
    int option_rv= getopt_long(argc, argv, "Vhvds:", long_options, &option_index);

    if (option_rv == -1) break;

    switch (option_rv)
    {
    case 0:
      break;

    case OPT_BINARY:
      opt_binary= true;
      break;

    case OPT_VERBOSE: /* --verbose or -v */
      opt_verbose= OPT_VERBOSE;
      break;

    case OPT_DEBUG: /* --debug or -d */
      opt_verbose= OPT_DEBUG;
      break;

    case OPT_VERSION: /* --version or -V */
      opt_verbose= true;
      break;

    case OPT_HELP: /* --help or -h */
      opt_help= true;
      break;

    case OPT_SERVERS: /* --servers or -s */
      opt_servers= strdup(optarg);
      break;

    case OPT_HASH:
      opt_hash= strdup(optarg);
      break;

    case OPT_USERNAME:
       opt_username= optarg;
       break;

    case OPT_PASSWD:
       opt_passwd= optarg;
       break;

    case OPT_QUIET:
      close_stdio();
      break;

    case '?':
      /* getopt_long already printed an error message. */
      exit(1);
    default:
      abort();
    }
  }

  if (opt_version)
  {
    version_command(PROGRAM_NAME);
    exit(EXIT_SUCCESS);
  }

  if (opt_help)
  {
    help_command(PROGRAM_NAME, PROGRAM_DESCRIPTION, long_options, NULL);
    exit(EXIT_SUCCESS);
  }
}
