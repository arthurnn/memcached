#include "libmemcached/common.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <libmemcached/memcached.h>

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

int main(int argc, char *argv[])
{
  memcached_st *memc;
  char *string;
  size_t string_length;
  uint32_t flags;
  memcached_return rc;
  memcached_server_st *servers;

  options_parse(argc, argv);

  if (!opt_servers)
  {
    char *temp;

    if ((temp= getenv("MEMCACHED_SERVERS")))
      opt_servers= strdup(temp);
    else
    {
      fprintf(stderr, "No Servers provided\n");
      exit(1);
    }
  }

  memc= memcached_create(NULL);
  process_hash_option(memc, opt_hash);

  servers= memcached_servers_parse(opt_servers);

  memcached_server_push(memc, servers);
  memcached_server_list_free(servers);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL,
                         (uint64_t)opt_binary);

  if (!initialize_sasl(memc, opt_username, opt_passwd))
  {
    memcached_free(memc);
    return 1;
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
          printf("key: %s\nflags: ", argv[optind]);
        printf("%x\n", flags);
      }
      else 
      {
        if (opt_verbose)
          printf("key: %s\nflags: %x\nlength: %zu\nvalue: ",
                 argv[optind], flags, string_length);
        printf("%.*s\n", (int)string_length, string);
        free(string);
      }
    }
    else if (rc != MEMCACHED_NOTFOUND)
    {
      fprintf(stderr, "memcat: %s: memcache error %s", 
              argv[optind], memcached_strerror(memc, rc));
      if (memc->cached_errno)
	fprintf(stderr, " system error %s", strerror(memc->cached_errno));
      fprintf(stderr, "\n");
    }
    optind++;
  }

  memcached_free(memc);

  if (opt_servers)
    free(opt_servers);
  if (opt_hash)
    free(opt_hash);

  shutdown_sasl();

  return 0;
}


void options_parse(int argc, char *argv[])
{
  int option_index= 0;
  int option_rv;

  memcached_programs_help_st help_options[]=
  {
    {0},
  };

  static struct option long_options[]=
    {
      {(OPTIONSTRING)"version", no_argument, NULL, OPT_VERSION},
      {(OPTIONSTRING)"help", no_argument, NULL, OPT_HELP},
      {(OPTIONSTRING)"verbose", no_argument, &opt_verbose, OPT_VERBOSE},
      {(OPTIONSTRING)"debug", no_argument, &opt_verbose, OPT_DEBUG},
      {(OPTIONSTRING)"servers", required_argument, NULL, OPT_SERVERS},
      {(OPTIONSTRING)"flag", no_argument, &opt_displayflag, OPT_FLAG},
      {(OPTIONSTRING)"hash", required_argument, NULL, OPT_HASH},
      {(OPTIONSTRING)"binary", no_argument, NULL, OPT_BINARY},
      {(OPTIONSTRING)"username", required_argument, NULL, OPT_USERNAME},
      {(OPTIONSTRING)"password", required_argument, NULL, OPT_PASSWD},
      {0, 0, 0, 0},
    };

  while (1) 
  {
    option_rv= getopt_long(argc, argv, "Vhvds:", long_options, &option_index);
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
    case '?':
      /* getopt_long already printed an error message. */
      exit(1);
    default:
      abort();
    }
  }
}
