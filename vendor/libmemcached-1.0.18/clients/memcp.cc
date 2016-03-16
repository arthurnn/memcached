/* LibMemcached
 * Copyright (C) 2011-2013 Data Differential, http://datadifferential.com/
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
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>


#include <libmemcached-1.0/memcached.h>

#include "client_options.h"
#include "utilities.h"

#define PROGRAM_NAME "memcp"
#define PROGRAM_DESCRIPTION "Copy a set of files to a memcached cluster."

/* Prototypes */
static void options_parse(int argc, char *argv[]);

static bool opt_binary= false;
static bool opt_udp= false;
static bool opt_buffer= false;
static int opt_verbose= 0;
static char *opt_servers= NULL;
static char *opt_hash= NULL;
static int opt_method= OPT_SET;
static uint32_t opt_flags= 0;
static time_t opt_expires= 0;
static char *opt_username;
static char *opt_passwd;

static long strtol_wrapper(const char *nptr, int base, bool *error)
{
  long val;
  char *endptr;

  errno= 0;    /* To distinguish success/failure after call */
  val= strtol(nptr, &endptr, base);

  /* Check for various possible errors */

  if ((errno == ERANGE and (val == LONG_MAX or val == LONG_MIN))
      or (errno != 0 && val == 0))
  {
    *error= true;
    return 0;
  }

  if (endptr == nptr)
  {
    *error= true;
    return 0;
  }

  *error= false;
  return val;
}

int main(int argc, char *argv[])
{

  options_parse(argc, argv);

  if (optind >= argc)
  {
    fprintf(stderr, "Expected argument after options\n");
    exit(EXIT_FAILURE);
  }

  initialize_sockets();

  memcached_st *memc= memcached_create(NULL);

  if (opt_udp)
  {
    if (opt_verbose)
    {
      std::cout << "Enabling UDP" << std::endl;
    }

    if (memcached_failed(memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP, opt_udp)))
    {
      memcached_free(memc);
      std::cerr << "Could not enable UDP protocol." << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (opt_buffer)
  {
    if (opt_verbose)
    {
      std::cout << "Enabling MEMCACHED_BEHAVIOR_BUFFER_REQUESTS" << std::endl;
    }

    if (memcached_failed(memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, opt_buffer)))
    {
      memcached_free(memc);
      std::cerr << "Could not enable MEMCACHED_BEHAVIOR_BUFFER_REQUESTS." << std::endl;
      return EXIT_FAILURE;
    }
  }

  process_hash_option(memc, opt_hash);

  if (opt_servers == NULL)
  {
    char *temp;

    if ((temp= getenv("MEMCACHED_SERVERS")))
    {
      opt_servers= strdup(temp);
    }
#if 0
    else if (argc >= 1 and argv[--argc])
    {
      opt_servers= strdup(argv[argc]);
    }
#endif

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

  memcached_server_push(memc, servers);
  memcached_server_list_free(servers);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, opt_binary);
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

  int exit_code= EXIT_SUCCESS;
  while (optind < argc)
  {
    int fd= open(argv[optind], O_RDONLY);
    if (fd < 0)
    {
      if (opt_verbose)
      {
        std::cerr << "memcp " << argv[optind] << " " << strerror(errno) << std::endl;
        optind++;
      }
      exit_code= EXIT_FAILURE;
      continue;
    }

    struct stat sbuf;
    if (fstat(fd, &sbuf) == -1)
    {
      std::cerr << "memcp " << argv[optind] << " " << strerror(errno) << std::endl;
      optind++;
      exit_code= EXIT_FAILURE;
      continue;
    }

    char *ptr= rindex(argv[optind], '/');
    if (ptr)
    {
      ptr++;
    }
    else
    {
      ptr= argv[optind];
    }

    if (opt_verbose)
    {
      static const char *opstr[] = { "set", "add", "replace" };
      printf("op: %s\nsource file: %s\nlength: %lu\n"
	     "key: %s\nflags: %x\nexpires: %lu\n",
	     opstr[opt_method - OPT_SET], argv[optind], (unsigned long)sbuf.st_size,
	     ptr, opt_flags, (unsigned long)opt_expires);
    }

    // The file may be empty
    char *file_buffer_ptr= NULL;
    if (sbuf.st_size > 0)
    {
      if ((file_buffer_ptr= (char *)malloc(sizeof(char) * (size_t)sbuf.st_size)) == NULL)
      {
        std::cerr << "Error allocating file buffer(" << strerror(errno) << ")" << std::endl;
        close(fd);
        exit(EXIT_FAILURE);
      }

      ssize_t read_length;
      if ((read_length= ::read(fd, file_buffer_ptr, (size_t)sbuf.st_size)) == -1)
      {
        std::cerr << "Error while reading file " << file_buffer_ptr << " (" << strerror(errno) << ")" << std::endl;
        close(fd);
        free(file_buffer_ptr);
        exit(EXIT_FAILURE);
      }

      if (read_length != sbuf.st_size)
      {
        std::cerr << "Failure while reading file. Read length was not equal to stat() length" << std::endl;
        close(fd);
        free(file_buffer_ptr);
        exit(EXIT_FAILURE);
      }
    }

    memcached_return_t rc;
    if (opt_method == OPT_ADD)
    {
      rc= memcached_add(memc, ptr, strlen(ptr),
                        file_buffer_ptr, (size_t)sbuf.st_size,
			opt_expires, opt_flags);
    }
    else if (opt_method == OPT_REPLACE)
    {
      rc= memcached_replace(memc, ptr, strlen(ptr),
			    file_buffer_ptr, (size_t)sbuf.st_size,
			    opt_expires, opt_flags);
    }
    else
    {
      rc= memcached_set(memc, ptr, strlen(ptr),
                        file_buffer_ptr, (size_t)sbuf.st_size,
                        opt_expires, opt_flags);
    }

    if (memcached_failed(rc))
    {
      std::cerr << "Error occrrured during memcached_set(): " << memcached_last_error_message(memc) << std::endl;
      exit_code= EXIT_FAILURE;
    }

    ::free(file_buffer_ptr);
    ::close(fd);
    optind++;
  }

  if (opt_verbose)
  {
    std::cout << "Calling memcached_free()" << std::endl;
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
  memcached_programs_help_st help_options[]=
  {
    {0},
  };

  static struct option long_options[]=
    {
      {(OPTIONSTRING)"version", no_argument, NULL, OPT_VERSION},
      {(OPTIONSTRING)"help", no_argument, NULL, OPT_HELP},
      {(OPTIONSTRING)"quiet", no_argument, NULL, OPT_QUIET},
      {(OPTIONSTRING)"udp", no_argument, NULL, OPT_UDP},
      {(OPTIONSTRING)"buffer", no_argument, NULL, OPT_BUFFER},
      {(OPTIONSTRING)"verbose", no_argument, &opt_verbose, OPT_VERBOSE},
      {(OPTIONSTRING)"debug", no_argument, &opt_verbose, OPT_DEBUG},
      {(OPTIONSTRING)"servers", required_argument, NULL, OPT_SERVERS},
      {(OPTIONSTRING)"flag", required_argument, NULL, OPT_FLAG},
      {(OPTIONSTRING)"expire", required_argument, NULL, OPT_EXPIRE},
      {(OPTIONSTRING)"set",  no_argument, NULL, OPT_SET},
      {(OPTIONSTRING)"add",  no_argument, NULL, OPT_ADD},
      {(OPTIONSTRING)"replace",  no_argument, NULL, OPT_REPLACE},
      {(OPTIONSTRING)"hash", required_argument, NULL, OPT_HASH},
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

    if (option_rv == -1)
      break;

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
      opt_version= true;
      break;

    case OPT_HELP: /* --help or -h */
      opt_help= true;
      break;

    case OPT_SERVERS: /* --servers or -s */
      opt_servers= strdup(optarg);
      break;

    case OPT_FLAG: /* --flag */
      {
        bool strtol_error;
        opt_flags= (uint32_t)strtol_wrapper(optarg, 16, &strtol_error);
        if (strtol_error == true)
        {
          fprintf(stderr, "Bad value passed via --flag\n");
          exit(1);
        }
      }
      break;

    case OPT_EXPIRE: /* --expire */
      {
        bool strtol_error;
        opt_expires= (time_t)strtol_wrapper(optarg, 10, &strtol_error);
        if (strtol_error == true)
        {
          fprintf(stderr, "Bad value passed via --expire\n");
          exit(1);
        }
      }
      break;

    case OPT_SET:
      opt_method= OPT_SET;
      break;

    case OPT_REPLACE:
      opt_method= OPT_REPLACE;
      break;

    case OPT_ADD:
      opt_method= OPT_ADD;
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

    case OPT_UDP:
      opt_udp= true;
      break;

    case OPT_BUFFER:
      opt_buffer= true;
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
