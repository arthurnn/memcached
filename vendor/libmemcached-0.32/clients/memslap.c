#include "libmemcached/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>
#include <getopt.h>
#include <pthread.h>
#include <assert.h>

#include <libmemcached/memcached.h>

#include "client_options.h"
#include "utilities.h"
#include "generator.h"
#include "execute.h"

#define DEFAULT_INITIAL_LOAD 10000
#define DEFAULT_EXECUTE_NUMBER 10000
#define DEFAULT_CONCURRENCY 1

#define PROGRAM_NAME "memslap"
#define PROGRAM_DESCRIPTION "Generates a load against a memcached custer of servers."

/* Global Thread counter */
volatile unsigned int thread_counter;
pthread_mutex_t counter_mutex;
pthread_cond_t count_threshhold;
volatile unsigned int master_wakeup;
pthread_mutex_t sleeper_mutex;
pthread_cond_t sleep_threshhold;

void *run_task(void *p);

/* Types */
typedef struct conclusions_st conclusions_st;
typedef struct thread_context_st thread_context_st;
typedef enum {
  SET_TEST,
  GET_TEST,
} test_type;

struct thread_context_st {
  unsigned int key_count;
  pairs_st *initial_pairs;
  unsigned int initial_number;
  pairs_st *execute_pairs;
  unsigned int execute_number;
  test_type test;
  memcached_st *memc;
};

struct conclusions_st {
  long int load_time;
  long int read_time;
  unsigned int rows_loaded;
  unsigned int rows_read;
};

/* Prototypes */
void options_parse(int argc, char *argv[]);
void conclusions_print(conclusions_st *conclusion);
void scheduler(memcached_server_st *servers, conclusions_st *conclusion);
pairs_st *load_create_data(memcached_st *memc, unsigned int number_of,
                           unsigned int *actual_loaded);
void flush_all(memcached_st *memc);

static int opt_binary= 0;
static int opt_verbose= 0;
static int opt_flush= 0;
static int opt_non_blocking_io= 0;
static int opt_tcp_nodelay= 0;
static unsigned int opt_execute_number= 0;
static unsigned int opt_createial_load= 0;
static unsigned int opt_concurrency= 0;
static int opt_displayflag= 0;
static char *opt_servers= NULL;
static int opt_udp_io= 0;
static char *opt_username;
static char *opt_passwd;

test_type opt_test= SET_TEST;

int main(int argc, char *argv[])
{
  conclusions_st conclusion;
  memcached_server_st *servers;

  memset(&conclusion, 0, sizeof(conclusions_st));

  srandom((unsigned int)time(NULL));
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

  servers= memcached_servers_parse(opt_servers);

  pthread_mutex_init(&counter_mutex, NULL);
  pthread_cond_init(&count_threshhold, NULL);
  pthread_mutex_init(&sleeper_mutex, NULL);
  pthread_cond_init(&sleep_threshhold, NULL);

  scheduler(servers, &conclusion);

  free(opt_servers);

  (void)pthread_mutex_destroy(&counter_mutex);
  (void)pthread_cond_destroy(&count_threshhold);
  (void)pthread_mutex_destroy(&sleeper_mutex);
  (void)pthread_cond_destroy(&sleep_threshhold);
  conclusions_print(&conclusion);
  memcached_server_list_free(servers);

  return 0;
}

void scheduler(memcached_server_st *servers, conclusions_st *conclusion)
{
  unsigned int x;
  unsigned int actual_loaded= 0; /* Fix warning */
  memcached_st *memc;

  struct timeval start_time, end_time;
  pthread_t mainthread;            /* Thread descriptor */
  pthread_attr_t attr;          /* Thread attributes */
  pairs_st *pairs= NULL;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,
                              PTHREAD_CREATE_DETACHED);

  memc= memcached_create(NULL);

  /* We need to set udp behavior before adding servers to the client */
  if (opt_udp_io)
  {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP,
                           (uint64_t)opt_udp_io);
    for(x= 0; x < servers[0].count; x++ )
      servers[x].type= MEMCACHED_CONNECTION_UDP;
  }
  memcached_server_push(memc, servers);

  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL,
                         (uint64_t)opt_binary);
  if (!initialize_sasl(memc, opt_username, opt_passwd))
  {
    memcached_free(memc);
    exit(1);
  }

  if (opt_flush)
    flush_all(memc);
  if (opt_createial_load)
    pairs= load_create_data(memc, opt_createial_load, &actual_loaded);

  /* We set this after we have loaded */
  {
    if (opt_non_blocking_io)
      memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1);
    if (opt_tcp_nodelay)
      memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, 1);
  }


  pthread_mutex_lock(&counter_mutex);
  thread_counter= 0;

  pthread_mutex_lock(&sleeper_mutex);
  master_wakeup= 1;
  pthread_mutex_unlock(&sleeper_mutex);

  for (x= 0; x < opt_concurrency; x++)
  {
    thread_context_st *context;
    context= (thread_context_st *)calloc(1, sizeof(thread_context_st));

    context->memc= memcached_clone(NULL, memc);
    context->test= opt_test;

    context->initial_pairs= pairs;
    context->initial_number= actual_loaded;

    if (opt_test == SET_TEST)
    {
      context->execute_pairs= pairs_generate(opt_execute_number, 400);
      context->execute_number= opt_execute_number;
    }

    /* now you create the thread */
    if (pthread_create(&mainthread, &attr, run_task,
                       (void *)context) != 0)
    {
      fprintf(stderr,"Could not create thread\n");
      exit(1);
    }
    thread_counter++;
  }

  pthread_mutex_unlock(&counter_mutex);
  pthread_attr_destroy(&attr);

  pthread_mutex_lock(&sleeper_mutex);
  master_wakeup= 0;
  pthread_mutex_unlock(&sleeper_mutex);
  pthread_cond_broadcast(&sleep_threshhold);

  gettimeofday(&start_time, NULL);
  /*
    We loop until we know that all children have cleaned up.
  */
  pthread_mutex_lock(&counter_mutex);
  while (thread_counter)
    pthread_cond_wait(&count_threshhold, &counter_mutex);
  pthread_mutex_unlock(&counter_mutex);

  gettimeofday(&end_time, NULL);

  conclusion->load_time= timedif(end_time, start_time);
  conclusion->read_time= timedif(end_time, start_time);
  pairs_free(pairs);
  memcached_free(memc);
  shutdown_sasl();
}


void options_parse(int argc, char *argv[])
{
  memcached_programs_help_st help_options[]=
  {
    {0},
  };

  static struct option long_options[]=
    {
      {(OPTIONSTRING)"concurrency", required_argument, NULL, OPT_SLAP_CONCURRENCY},
      {(OPTIONSTRING)"debug", no_argument, &opt_verbose, OPT_DEBUG},
      {(OPTIONSTRING)"execute-number", required_argument, NULL, OPT_SLAP_EXECUTE_NUMBER},
      {(OPTIONSTRING)"flag", no_argument, &opt_displayflag, OPT_FLAG},
      {(OPTIONSTRING)"flush", no_argument, &opt_flush, OPT_FLUSH},
      {(OPTIONSTRING)"help", no_argument, NULL, OPT_HELP},
      {(OPTIONSTRING)"initial-load", required_argument, NULL, OPT_SLAP_INITIAL_LOAD}, /* Number to load initially */
      {(OPTIONSTRING)"non-blocking", no_argument, &opt_non_blocking_io, OPT_SLAP_NON_BLOCK},
      {(OPTIONSTRING)"servers", required_argument, NULL, OPT_SERVERS},
      {(OPTIONSTRING)"tcp-nodelay", no_argument, &opt_tcp_nodelay, OPT_SLAP_TCP_NODELAY},
      {(OPTIONSTRING)"test", required_argument, NULL, OPT_SLAP_TEST},
      {(OPTIONSTRING)"verbose", no_argument, &opt_verbose, OPT_VERBOSE},
      {(OPTIONSTRING)"version", no_argument, NULL, OPT_VERSION},
      {(OPTIONSTRING)"binary", no_argument, NULL, OPT_BINARY},
      {(OPTIONSTRING)"udp", no_argument, NULL, OPT_UDP},
      {(OPTIONSTRING)"username", required_argument, NULL, OPT_USERNAME},
      {(OPTIONSTRING)"password", required_argument, NULL, OPT_PASSWD},
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
    case OPT_UDP:
      if (opt_test == GET_TEST)
      {
        fprintf(stderr, "You can not run a get test in UDP mode. UDP mode "
                  "does not currently support get ops.\n");
        exit(1);
      }
      opt_udp_io= 1;
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
    case OPT_SLAP_TEST:
      if (!strcmp(optarg, "get"))
      {
        if (opt_udp_io == 1)
        {
          fprintf(stderr, "You can not run a get test in UDP mode. UDP mode "
                  "does not currently support get ops.\n");
          exit(1);
        }
        opt_test= GET_TEST ;
      }
      else if (!strcmp(optarg, "set"))
        opt_test= SET_TEST;
      else
      {
        fprintf(stderr, "Your test, %s, is not a known test\n", optarg);
        exit(1);
      }
      break;
    case OPT_SLAP_CONCURRENCY:
      opt_concurrency= (unsigned int)strtoul(optarg, (char **)NULL, 10);
      break;
    case OPT_SLAP_EXECUTE_NUMBER:
      opt_execute_number= (unsigned int)strtoul(optarg, (char **)NULL, 10);
      break;
    case OPT_SLAP_INITIAL_LOAD:
      opt_createial_load= (unsigned int)strtoul(optarg, (char **)NULL, 10);
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

  if (opt_test == GET_TEST && opt_createial_load == 0)
    opt_createial_load= DEFAULT_INITIAL_LOAD;

  if (opt_execute_number == 0)
    opt_execute_number= DEFAULT_EXECUTE_NUMBER;

  if (opt_concurrency == 0)
    opt_concurrency= DEFAULT_CONCURRENCY;
}

void conclusions_print(conclusions_st *conclusion)
{
  printf("\tThreads connecting to servers %u\n", opt_concurrency);
#ifdef NOT_FINISHED
  printf("\tLoaded %u rows\n", conclusion->rows_loaded);
  printf("\tRead %u rows\n", conclusion->rows_read);
#endif
  if (opt_test == SET_TEST)
    printf("\tTook %ld.%03ld seconds to load data\n", conclusion->load_time / 1000,
           conclusion->load_time % 1000);
  else
    printf("\tTook %ld.%03ld seconds to read data\n", conclusion->read_time / 1000,
           conclusion->read_time % 1000);
}

void *run_task(void *p)
{
  thread_context_st *context= (thread_context_st *)p;
  memcached_st *memc;

  memc= context->memc;

  pthread_mutex_lock(&sleeper_mutex);
  while (master_wakeup)
  {
    pthread_cond_wait(&sleep_threshhold, &sleeper_mutex);
  }
  pthread_mutex_unlock(&sleeper_mutex);

  /* Do Stuff */
  switch (context->test)
  {
  case SET_TEST:
    assert(context->execute_pairs);
    execute_set(memc, context->execute_pairs, context->execute_number);
    break;
  case GET_TEST:
    execute_get(memc, context->initial_pairs, context->initial_number);
    break;
  default:
    WATCHPOINT_ASSERT(context->test);
    break;
  }

  memcached_free(memc);

  if (context->execute_pairs)
    pairs_free(context->execute_pairs);

  free(context);

  pthread_mutex_lock(&counter_mutex);
  thread_counter--;
  pthread_cond_signal(&count_threshhold);
  pthread_mutex_unlock(&counter_mutex);

  return NULL;
}

void flush_all(memcached_st *memc)
{
  memcached_flush(memc, 0);
}

pairs_st *load_create_data(memcached_st *memc, unsigned int number_of,
                           unsigned int *actual_loaded)
{
  memcached_st *memc_clone;
  pairs_st *pairs;

  memc_clone= memcached_clone(NULL, memc);
  /* We always used non-blocking IO for load since it is faster */
  memcached_behavior_set(memc_clone, MEMCACHED_BEHAVIOR_NO_BLOCK, 0);

  pairs= pairs_generate(number_of, 400);
  *actual_loaded= execute_set(memc_clone, pairs, number_of);

  memcached_free(memc_clone);

  return pairs;
}
