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

#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <libmemcached-1.0/memcached.h>

#include "utilities.h"

#define PROGRAM_NAME "memcat"
#define PROGRAM_DESCRIPTION "Cat a set of key values to stdout."


/* Prototypes */
void options_parse(int argc, char *argv[]);

static int opt_binary= 0;
static int opt_verbose= 0;
static int opt_displayflag= 0;
static char *opt_servers= NULL;
static char *opt_hash= NULL;
static char *opt_username;
static char *opt_passwd;
static char *opt_file;

int main(int argc, char *argv[])
{
  char *string;
  size_t string_length;
  uint32_t flags;
  memcached_return_t rc;

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
      std::cerr << "No servers provied" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  memcached_server_st* servers= memcached_servers_parse(opt_servers);
  if (servers == NULL or memcached_server_list_count(servers) == 0)
  {
    std::cerr << "Invalid server list provided:" << opt_servers << std::endl;
    return EXIT_FAILURE;
  }

  memcached_st* memc= memcached_create(NULL);
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
    string= memcached_get(memc, argv[optind], strlen(argv[optind]),
                          &string_length, &flags, &rc);
    if (rc == MEMCACHED_SUCCESS)
    {
      if (opt_displayflag)
      {
        if (opt_verbose)
        {
          std::cout << "key: " << argv[optind] << std::endl << "flags: " << flags << std::endl;
        }
      }
      else
      {
        if (opt_verbose)
        {
          std::cout << "key: " << argv[optind] << std::endl << "flags: " << flags << "length: " << string_length << std::endl << "value: ";
        }

        if (opt_file)
        {
          FILE *fp= fopen(opt_file, "w");
          if (fp == NULL)
          {
            perror("fopen");
            return_code= EXIT_FAILURE;
            break;
          }

          size_t written= fwrite(string, 1, string_length, fp);
          if (written != string_length) 
          {
            std::cerr << "error writing file to file " << opt_file << " wrote " << written << ", should have written" << string_length << std::endl;
            return_code= EXIT_FAILURE;
            break;
          }

          if (fclose(fp))
          {
            std::cerr << "error closing " << opt_file << std::endl;
            return_code= EXIT_FAILURE;
            break;
          }
        }
        else
        {
          std::cout.write(string, string_length);
          std::cout << std::endl;
        }
        free(string);
      }
    }
    else if (rc != MEMCACHED_NOTFOUND)
    {
      std::cerr << "error on " << argv[optind] << "(" <<  memcached_strerror(memc, rc) << ")";
      if (memcached_last_error_errno(memc))
      {
        std::cerr << " system error (" << strerror(memcached_last_error_errno(memc)) << ")" << std::endl;
      }
      std::cerr << std::endl;

      return_code= EXIT_FAILURE;
      break;
    }
    else // Unknown Issue
    {
      std::cerr << "error on " << argv[optind] << "("<< memcached_strerror(NULL, rc) << ")" << std::endl;
      return_code= EXIT_FAILURE;
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
  int option_index= 0;

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
      {(OPTIONSTRING)"flag", no_argument, &opt_displayflag, OPT_FLAG},
      {(OPTIONSTRING)"hash", required_argument, NULL, OPT_HASH},
      {(OPTIONSTRING)"binary", no_argument, NULL, OPT_BINARY},
      {(OPTIONSTRING)"username", required_argument, NULL, OPT_USERNAME},
      {(OPTIONSTRING)"password", required_argument, NULL, OPT_PASSWD},
      {(OPTIONSTRING)"file", required_argument, NULL, OPT_FILE},
      {0, 0, 0, 0},
    };

  while (1)
  {
    int option_rv= getopt_long(argc, argv, "Vhvds:", long_options, &option_index);
    if (option_rv == -1) break;
    switch (option_rv)
    {
    case 0:
      break;
    case OPT_BINARY:
      opt_binary = 1;
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
    case OPT_HASH:
      opt_hash= strdup(optarg);
      break;
    case OPT_USERNAME:
      opt_username= optarg;
      break;
    case OPT_PASSWD:
      opt_passwd= optarg;
      break;
    case OPT_FILE:
      opt_file= optarg;
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
}
