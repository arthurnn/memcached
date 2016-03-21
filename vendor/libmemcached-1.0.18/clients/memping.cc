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
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include <libmemcached-1.0/memcached.h>
#include <libmemcachedutil-1.0/util.h>
#include "client_options.h"
#include "utilities.h"

#include <iostream>

static bool opt_binary= false;
static int opt_verbose= 0;
static time_t opt_expire= 0;
static char *opt_servers= NULL;
static char *opt_username;
static char *opt_passwd;

#define PROGRAM_NAME "memping"
#define PROGRAM_DESCRIPTION "Ping a server to see if it is alive"

/* Prototypes */
void options_parse(int argc, char *argv[]);

int main(int argc, char *argv[])
{
  options_parse(argc, argv);

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

  int exit_code= EXIT_SUCCESS;
  memcached_server_st *servers= memcached_servers_parse(opt_servers);
  if (servers == NULL or memcached_server_list_count(servers) == 0)
  {
    std::cerr << "Invalid server list provided:" << opt_servers << std::endl;
    exit_code= EXIT_FAILURE;
  }
  else
  {
    for (uint32_t x= 0; x < memcached_server_list_count(servers); x++)
    {
      memcached_return_t instance_rc;
      const char *hostname= servers[x].hostname;
      in_port_t port= servers[x].port;

      if (opt_verbose)
      {
        std::cout << "Trying to ping " << hostname << ":" << port << std::endl;
      }

      if (libmemcached_util_ping2(hostname, port, opt_username, opt_passwd, &instance_rc) == false)
      {
        std::cerr << "Failed to ping " << hostname << ":" << port << " " << memcached_strerror(NULL, instance_rc) <<  std::endl;
        exit_code= EXIT_FAILURE;
      }
    }
  }
  memcached_server_list_free(servers);

  free(opt_servers);

  return exit_code;
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
    {(OPTIONSTRING)"expire", required_argument, NULL, OPT_EXPIRE},
    {(OPTIONSTRING)"binary", no_argument, NULL, OPT_BINARY},
    {(OPTIONSTRING)"username", required_argument, NULL, OPT_USERNAME},
    {(OPTIONSTRING)"password", required_argument, NULL, OPT_PASSWD},
    {0, 0, 0, 0},
  };

  bool opt_version= false;
  bool opt_help= false;
  int option_index= 0;
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
      opt_verbose = OPT_VERBOSE;
      break;

    case OPT_DEBUG: /* --debug or -d */
      opt_verbose = OPT_DEBUG;
      break;

    case OPT_VERSION: /* --version or -V */
      version_command(PROGRAM_NAME);
      break;

    case OPT_HELP: /* --help or -h */
      help_command(PROGRAM_NAME, PROGRAM_DESCRIPTION, long_options, help_options);
      break;

    case OPT_SERVERS: /* --servers or -s */
      opt_servers= strdup(optarg);
      break;

    case OPT_EXPIRE: /* --expire */
      errno= 0;
      opt_expire= time_t(strtoll(optarg, (char **)NULL, 10));
      if (errno != 0)
      {
        std::cerr << "Incorrect value passed to --expire: `" << optarg << "`" << std::endl;
        exit(EXIT_FAILURE);
      }
      break;

    case OPT_USERNAME:
      opt_username= optarg;
      opt_binary= true;
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
    help_command(PROGRAM_NAME, PROGRAM_DESCRIPTION, long_options, help_options);
    exit(EXIT_SUCCESS);
  }
}
