#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>

#include <libmemcached/memcached.h>

#include "client_options.h"
#include "utilities.h"

#define PROGRAM_NAME "memstat"
#define PROGRAM_DESCRIPTION "Output the state of a memcached cluster."

/* Prototypes */
void options_parse(int argc, char *argv[]);

static int opt_verbose= 0;
static int opt_displayflag= 0;
static char *opt_servers= NULL;

int main(int argc, char *argv[])
{
  unsigned int x;
  memcached_return rc;
  memcached_st *memc;
  memcached_stat_st *stat;
  memcached_server_st *servers;
  memcached_server_st *server_list;

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

  servers= memcached_servers_parse(opt_servers);
  memcached_server_push(memc, servers);
  memcached_server_list_free(servers);

  stat= memcached_stat(memc, NULL, &rc);

  if (rc != MEMCACHED_SUCCESS && rc != MEMCACHED_SOME_ERRORS)
  {
    printf("Failure to communicate with servers (%s)\n",
	   memcached_strerror(memc, rc));
    exit(1);
  }

  server_list= memcached_server_list(memc);

  printf("Listing %u Server\n\n", memcached_server_count(memc));
  for (x= 0; x < memcached_server_count(memc); x++)
  {
    char **list;
    char **ptr;

    list= memcached_stat_get_keys(memc, &stat[x], &rc);

    printf("Server: %s (%u)\n", memcached_server_name(memc, server_list[x]),
	   memcached_server_port(memc, server_list[x]));
    for (ptr= list; *ptr; ptr++)
    {
      memcached_return rc;
      char *value= memcached_stat_get_value(memc, &stat[x], *ptr, &rc);

      printf("\t %s: %s\n", *ptr, value);
      free(value);
    }

    free(list);
    printf("\n");
  }

  free(stat);
  free(opt_servers);

  memcached_free(memc);

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
    {"flag", no_argument, &opt_displayflag, OPT_FLAG},
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
    case '?':
      /* getopt_long already printed an error message. */
      exit(1);
    default:
      abort();
    }
  }
}
