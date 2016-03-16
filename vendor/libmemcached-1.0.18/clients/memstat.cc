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
 * Authors: 
 *          Brian Aker
 *          Toru Maesaka
 */
#include <mem_config.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>

#include <libmemcached-1.0/memcached.h>

#include "client_options.h"
#include "utilities.h"

#define PROGRAM_NAME "memstat"
#define PROGRAM_DESCRIPTION "Output the state of a memcached cluster."

/* Prototypes */
static void options_parse(int argc, char *argv[]);
static void run_analyzer(memcached_st *memc, memcached_stat_st *memc_stat);
static void print_analysis_report(memcached_st *memc,
                                  memcached_analysis_st *report);

static bool opt_binary= false;
static bool opt_verbose= false;
static bool opt_server_version= false;
static bool opt_analyze= false;
static char *opt_servers= NULL;
static char *stat_args= NULL;
static char *analyze_mode= NULL;
static char *opt_username;
static char *opt_passwd;

static struct option long_options[]=
{
  {(OPTIONSTRING)"args", required_argument, NULL, OPT_STAT_ARGS},
  {(OPTIONSTRING)"version", no_argument, NULL, OPT_VERSION},
  {(OPTIONSTRING)"help", no_argument, NULL, OPT_HELP},
  {(OPTIONSTRING)"quiet", no_argument, NULL, OPT_QUIET},
  {(OPTIONSTRING)"verbose", no_argument, NULL, OPT_VERBOSE},
  {(OPTIONSTRING)"binary", no_argument, NULL, OPT_BINARY},
  {(OPTIONSTRING)"debug", no_argument, NULL, OPT_DEBUG},
  {(OPTIONSTRING)"server-version", no_argument, NULL, OPT_SERVER_VERSION},
  {(OPTIONSTRING)"servers", required_argument, NULL, OPT_SERVERS},
  {(OPTIONSTRING)"analyze", optional_argument, NULL, OPT_ANALYZE},
  {(OPTIONSTRING)"username", required_argument, NULL, OPT_USERNAME},
  {(OPTIONSTRING)"password", required_argument, NULL, OPT_PASSWD},
  {0, 0, 0, 0},
};


static memcached_return_t stat_printer(const memcached_instance_st * instance,
                                       const char *key, size_t key_length,
                                       const char *value, size_t value_length,
                                       void *context)
{
  static const memcached_instance_st * last= NULL;
  (void)context;

  if (last != instance)
  {
    printf("Server: %s (%u)\n", memcached_server_name(instance),
           (uint32_t)memcached_server_port(instance));
    last= instance;
  }

  printf("\t %.*s: %.*s\n", (int)key_length, key, (int)value_length, value);

  return MEMCACHED_SUCCESS;
}

static memcached_return_t server_print_callback(const memcached_st *,
                                                const memcached_instance_st * instance,
                                                void *)
{
  std::cerr << memcached_server_name(instance) << ":" << memcached_server_port(instance) <<
    " " << int(memcached_server_major_version(instance)) << 
    "." << int(memcached_server_minor_version(instance)) << 
    "." << int(memcached_server_micro_version(instance)) << std::endl;

  return MEMCACHED_SUCCESS;
}

int main(int argc, char *argv[])
{
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
      return EXIT_FAILURE;
    }
  }

  memcached_server_st* servers= memcached_servers_parse(opt_servers);
  if (servers == NULL or memcached_server_list_count(servers) == 0)
  {
    std::cerr << "Invalid server list provided:" << opt_servers << std::endl;
    return EXIT_FAILURE;
  }
  
  if (opt_servers)
  {
    free(opt_servers);
  }

  memcached_st *memc= memcached_create(NULL);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, opt_binary);

  memcached_return_t rc= memcached_server_push(memc, servers);
  memcached_server_list_free(servers);

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

  if (rc != MEMCACHED_SUCCESS and rc != MEMCACHED_SOME_ERRORS)
  {
    printf("Failure to communicate with servers (%s)\n",
           memcached_strerror(memc, rc));
    exit(EXIT_FAILURE);
  }

  if (opt_server_version)
  {
    if (memcached_failed(memcached_version(memc)))
    {
      std::cerr << "Unable to obtain server version";
      exit(EXIT_FAILURE);
    }

    memcached_server_fn callbacks[1];
    callbacks[0]= server_print_callback;
    memcached_server_cursor(memc, callbacks, NULL,  1);
  }
  else if (opt_analyze)
  {
    memcached_stat_st *memc_stat= memcached_stat(memc, NULL, &rc);

    if (memc_stat == NULL)
    {
      exit(EXIT_FAILURE);
    }

    run_analyzer(memc, memc_stat);

    memcached_stat_free(memc, memc_stat);
  }
  else
  {
    rc= memcached_stat_execute(memc, stat_args, stat_printer, NULL);
  }

  memcached_free(memc);

  return rc == MEMCACHED_SUCCESS ? EXIT_SUCCESS: EXIT_FAILURE;
}

static void run_analyzer(memcached_st *memc, memcached_stat_st *memc_stat)
{
  memcached_return_t rc;

  if (analyze_mode == NULL)
  {
    memcached_analysis_st *report;
    report= memcached_analyze(memc, memc_stat, &rc);
    if (rc != MEMCACHED_SUCCESS || report == NULL)
    {
      printf("Failure to analyze servers (%s)\n",
             memcached_strerror(memc, rc));
      exit(1);
    }
    print_analysis_report(memc, report);
    free(report);
  }
  else if (strcmp(analyze_mode, "latency") == 0)
  {
    uint32_t flags, server_count= memcached_server_count(memc);
    uint32_t num_of_tests= 32;
    const char *test_key= "libmemcached_test_key";

    memcached_st **servers= static_cast<memcached_st**>(malloc(sizeof(memcached_st*) * server_count));
    if (servers == NULL)
    {
      fprintf(stderr, "Failed to allocate memory\n");
      return;
    }

    for (uint32_t x= 0; x < server_count; x++)
    {
      const memcached_instance_st * instance=
        memcached_server_instance_by_position(memc, x);

      if ((servers[x]= memcached_create(NULL)) == NULL)
      {
        fprintf(stderr, "Failed to memcached_create()\n");
        if (x > 0)
        {
          memcached_free(servers[0]);
        }
        x--;

        for (; x > 0; x--)
        {
          memcached_free(servers[x]);
        }

        free(servers);

        return;
      }
      memcached_server_add(servers[x],
                           memcached_server_name(instance),
                           memcached_server_port(instance));
    }

    printf("Network Latency Test:\n\n");
    struct timeval start_time, end_time;
    uint32_t slowest_server= 0;
    long elapsed_time, slowest_time= 0;

    for (uint32_t x= 0; x < server_count; x++)
    {
      const memcached_instance_st * instance=
        memcached_server_instance_by_position(memc, x);
      gettimeofday(&start_time, NULL);

      for (uint32_t y= 0; y < num_of_tests; y++)
      {
        size_t vlen;
        char *val= memcached_get(servers[x], test_key, strlen(test_key),
                                 &vlen, &flags, &rc);
        if (rc != MEMCACHED_NOTFOUND and rc != MEMCACHED_SUCCESS)
        {
          break;
        }
        free(val);
      }
      gettimeofday(&end_time, NULL);

      elapsed_time= (long) timedif(end_time, start_time);
      elapsed_time /= (long) num_of_tests;

      if (elapsed_time > slowest_time)
      {
        slowest_server= x;
        slowest_time= elapsed_time;
      }

      if (rc != MEMCACHED_NOTFOUND && rc != MEMCACHED_SUCCESS)
      {
        printf("\t %s (%d)  =>  failed to reach the server\n",
               memcached_server_name(instance),
               memcached_server_port(instance));
      }
      else
      {
        printf("\t %s (%d)  =>  %ld.%ld seconds\n",
               memcached_server_name(instance),
               memcached_server_port(instance),
               elapsed_time / 1000, elapsed_time % 1000);
      }
    }

    if (server_count > 1 && slowest_time > 0)
    {
      const memcached_instance_st * slowest=
        memcached_server_instance_by_position(memc, slowest_server);

      printf("---\n");
      printf("Slowest Server: %s (%d) => %ld.%ld seconds\n",
             memcached_server_name(slowest),
             memcached_server_port(slowest),
             slowest_time / 1000, slowest_time % 1000);
    }
    printf("\n");

    for (uint32_t x= 0; x < server_count; x++)
    {
      memcached_free(servers[x]);
    }

    free(servers);
    free(analyze_mode);
  }
  else
  {
    fprintf(stderr, "Invalid Analyzer Option provided\n");
    free(analyze_mode);
  }
}

static void print_analysis_report(memcached_st *memc,
                                  memcached_analysis_st *report)
                                  
{
  uint32_t server_count= memcached_server_count(memc);
  const memcached_instance_st * most_consumed_server= memcached_server_instance_by_position(memc, report->most_consumed_server);
  const memcached_instance_st * least_free_server= memcached_server_instance_by_position(memc, report->least_free_server);
  const memcached_instance_st * oldest_server= memcached_server_instance_by_position(memc, report->oldest_server);

  printf("Memcached Cluster Analysis Report\n\n");

  printf("\tNumber of Servers Analyzed         : %u\n", server_count);
  printf("\tAverage Item Size (incl/overhead)  : %u bytes\n",
         report->average_item_size);

  if (server_count == 1)
  {
    printf("\nFor a detailed report, you must supply multiple servers.\n");
    return;
  }

  printf("\n");
  printf("\tNode with most memory consumption  : %s:%u (%llu bytes)\n",
         memcached_server_name(most_consumed_server),
         (uint32_t)memcached_server_port(most_consumed_server),
         (unsigned long long)report->most_used_bytes);
  printf("\tNode with least free space         : %s:%u (%llu bytes remaining)\n",
         memcached_server_name(least_free_server),
         (uint32_t)memcached_server_port(least_free_server),
         (unsigned long long)report->least_remaining_bytes);
  printf("\tNode with longest uptime           : %s:%u (%us)\n",
         memcached_server_name(oldest_server),
         (uint32_t)memcached_server_port(oldest_server),
         report->longest_uptime);
  printf("\tPool-wide Hit Ratio                : %1.f%%\n", report->pool_hit_ratio);
  printf("\n");
}

static void options_parse(int argc, char *argv[])
{
  memcached_programs_help_st help_options[]=
  {
    {0},
  };

  int option_index= 0;

  bool opt_version= false;
  bool opt_help= false;
  while (1) 
  {
    int option_rv= getopt_long(argc, argv, "Vhvds:a", long_options, &option_index);

    if (option_rv == -1)
      break;

    switch (option_rv)
    {
    case 0:
      break;

    case OPT_VERBOSE: /* --verbose or -v */
      opt_verbose= true;
      break;

    case OPT_DEBUG: /* --debug or -d */
      opt_verbose= true;
      break;

    case OPT_BINARY:
      opt_binary= true;
      break;

    case OPT_SERVER_VERSION:
      opt_server_version= true;
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

    case OPT_STAT_ARGS:
      stat_args= strdup(optarg);
      break;

    case OPT_ANALYZE: /* --analyze or -a */
      opt_analyze= OPT_ANALYZE;
      analyze_mode= (optarg) ? strdup(optarg) : NULL;
      break;

    case OPT_QUIET:
      close_stdio();
      break;

    case OPT_USERNAME:
      opt_username= optarg;
      opt_binary= true;
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
