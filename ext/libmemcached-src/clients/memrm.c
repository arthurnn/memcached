#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <libmemcached/memcached.h>
#include <string.h>
#include "client_options.h"
#include "utilities.h"

static int opt_binary= 0;
static int opt_verbose= 0;
static time_t opt_expire= 0;
static char *opt_servers= NULL;
static char *opt_hash= NULL;

#define PROGRAM_NAME "memrm"
#define PROGRAM_DESCRIPTION "Erase a key or set of keys from a memcached cluster."

/* Prototypes */
void options_parse(int argc, char *argv[]);

int main(int argc, char *argv[])
{
  memcached_st *memc;
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
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, opt_binary);
  
  while (optind < argc) 
  {
    if (opt_verbose) 
      printf("key: %s\nexpires: %llu\n", argv[optind], (unsigned long long)opt_expire);
    rc = memcached_delete(memc, argv[optind], strlen(argv[optind]), opt_expire);

    if (rc != MEMCACHED_SUCCESS) 
    {
      fprintf(stderr, "memrm: %s: memcache error %s", 
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

  return 0;
}


void options_parse(int argc, char *argv[])
{
  memcached_programs_help_st help_options[]=
  {
    {0},
  };

  static struct option long_options[]=
  {
    {"version", no_argument, NULL, OPT_VERSION},
    {"help", no_argument, NULL, OPT_HELP},
    {"verbose", no_argument, &opt_verbose, OPT_VERBOSE},
    {"debug", no_argument, &opt_verbose, OPT_DEBUG},
    {"servers", required_argument, NULL, OPT_SERVERS},
    {"expire", required_argument, NULL, OPT_EXPIRE},
    {"hash", required_argument, NULL, OPT_HASH},
    {"binary", no_argument, NULL, OPT_BINARY},
    {0, 0, 0, 0},
  };
  int option_index= 0;
  int option_rv;

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
    case OPT_EXPIRE: /* --expire */
      opt_expire= (time_t)strtoll(optarg, (char **)NULL, 10);
      break;
    case OPT_HASH:
      opt_hash= strdup(optarg);
      break;
    case '?':
      /* getopt_long already printed an error message. */
      exit(1);
    default:
      abort();
    }
  }
}
