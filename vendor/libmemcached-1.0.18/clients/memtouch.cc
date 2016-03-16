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

#include <mem_config.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <unistd.h>

#include <libmemcached-1.0/memcached.h>

#include "utilities.h"

#define PROGRAM_NAME "memtouch"
#define PROGRAM_DESCRIPTION "Update the expiration value of an already existing value in the server"


/* Prototypes */
void options_parse(int argc, char *argv[]);

static int opt_binary= 0;
static int opt_verbose= 0;
static char *opt_servers= NULL;
static char *opt_hash= NULL;
static char *opt_username;
static char *opt_passwd;

time_t expiration= 0;

int main(int argc, char *argv[])
{
  int return_code= EXIT_SUCCESS;

  options_parse(argc, argv);
  initialize_sockets();

  if (opt_servers == NULL)
  {
    char *temp;

    if ((temp= getenv("MEMCACHED_SERVERS")))
    {
      opt_servers= strdup(temp);
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

  while (optind < argc)
  {
    memcached_return_t rc= memcached_touch(memc, argv[optind], strlen(argv[optind]), expiration);
    if (rc == MEMCACHED_NOTFOUND)
    {
      if (opt_verbose)
      {
        std::cout << "Could not find key \"" << argv[optind] << "\"" << std::endl;
      }

      return_code= EXIT_FAILURE;
    }
    else if (memcached_failed(rc))
    {
      if (opt_verbose)
      {
        std::cerr << "Fatal error for key \"" << argv[optind] << "\" :" <<  memcached_last_error_message(memc) << std::endl;
      }

      return_code= EXIT_FAILURE;
    }
    else // success
    {
      if (opt_verbose)
      {
        std::cout << "Found key " << argv[optind] << std::endl;
      }
    }

    optind++;
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

  return return_code;
}


void options_parse(int argc, char *argv[])
{
  memcached_programs_help_st help_options[]=
  {
    {0},
  };

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
      {(OPTIONSTRING)"expire", required_argument, NULL, OPT_EXPIRE},
      {0, 0, 0, 0},
    };

  bool opt_version= false;
  bool opt_help= false;
  int option_index= 0;

  while (1)
  {
    int option_rv= getopt_long(argc, argv, "Vhvds:", long_options, &option_index);
    if (option_rv == -1) 
    {
      break;
    }

    switch (option_rv)
    {
    case 0:
      break;

    case OPT_BINARY:
      opt_binary = true;
      break;

    case OPT_VERBOSE: /* --verbose or -v */
      opt_verbose = OPT_VERBOSE;
      break;

    case OPT_DEBUG: /* --debug or -d */
      opt_verbose = OPT_DEBUG;
      break;

    case OPT_VERSION: /* --version or -V */
      opt_version= true;
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

    case OPT_EXPIRE:
      errno= 0;
      expiration= time_t(strtoul(optarg, (char **)NULL, 10));
      if (errno != 0)
      {
        fprintf(stderr, "Invalid value for --expire: %s\n", optarg);
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_QUIET:
      close_stdio();
      break;

    case '?':
      /* getopt_long already printed an error message. */
      exit(EXIT_FAILURE);

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
    help_command(PROGRAM_NAME, PROGRAM_DESCRIPTION, long_options, help_options);
    exit(EXIT_SUCCESS);
  }
}
