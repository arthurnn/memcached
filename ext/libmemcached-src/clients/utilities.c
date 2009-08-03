#include "libmemcached/common.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "utilities.h"


long int timedif(struct timeval a, struct timeval b)
{
  register int us, s;

  us = (int)(a.tv_usec - b.tv_usec);
  us /= 1000;
  s = (int)(a.tv_sec - b.tv_sec);
  s *= 1000;
  return s + us;
}

void version_command(const char *command_name)
{
  printf("%s v%u.%u\n", command_name, 1, 0);
  exit(0);
}

static const char *lookup_help(memcached_options option)
{
  switch (option)
  {
  case OPT_SERVERS: return("List which servers you wish to connect to.");
  case OPT_VERSION: return("Display the version of the application and then exit.");
  case OPT_HELP: return("Diplay this message and then exit.");
  case OPT_VERBOSE: return("Give more details on the progression of the application.");
  case OPT_DEBUG: return("Provide output only useful for debugging.");
  case OPT_FLAG: return("Provide flag information for storage operation.");
  case OPT_EXPIRE: return("Set the expire option for the object.");
  case OPT_SET: return("Use set command with memcached when storing.");
  case OPT_REPLACE: return("Use replace command with memcached when storing.");
  case OPT_ADD: return("Use add command with memcached when storing.");
  case OPT_SLAP_EXECUTE_NUMBER: return("Number of times to execute the given test.");
  case OPT_SLAP_INITIAL_LOAD: return("Number of key pairs to load before executing tests.");
  case OPT_SLAP_TEST: return("Test to run (currently \"get\" or \"set\").");
  case OPT_SLAP_CONCURRENCY: return("Number of users to simulate with load.");
  case OPT_SLAP_NON_BLOCK: return("Set TCP up to use non-blocking IO.");
  case OPT_SLAP_TCP_NODELAY: return("Set TCP socket up to use nodelay.");
  case OPT_FLUSH: return("Flush servers before running tests.");
  case OPT_HASH: return("Select hash type.");
  case OPT_BINARY: return("Switch to binary protocol.");
  case OPT_ANALYZE: return("Analyze the provided servers.");
  case OPT_UDP: return("Use UDP protocol when communicating with server.");
  default: WATCHPOINT_ASSERT(0);
  };

  WATCHPOINT_ASSERT(0);
  return "forgot to document this function :)";
}

void help_command(const char *command_name, const char *description,
                  const struct option *long_options,
                  memcached_programs_help_st *options __attribute__((unused)))
{
  unsigned int x;

  printf("%s v%u.%u\n\n", command_name, 1, 0);
  printf("\t%s\n\n", description);
  printf("Current options. A '=' means the option takes a value.\n\n");

  for (x= 0; long_options[x].name; x++) 
  {
    const char *help_message;

    printf("\t --%s%c\n", long_options[x].name, 
           long_options[x].has_arg ? '=' : ' ');  
    if ((help_message= lookup_help(long_options[x].val)))
      printf("\t\t%s\n", help_message);
  }

  printf("\n");
  exit(0);
}

void process_hash_option(memcached_st *memc, char *opt_hash)
{
  uint64_t set;
  memcached_return rc;

  if (opt_hash == NULL)
    return;

  set= MEMCACHED_HASH_DEFAULT; /* Just here to solve warning */
  if (!strcasecmp(opt_hash, "CRC"))
    set= MEMCACHED_HASH_CRC;
  else if (!strcasecmp(opt_hash, "FNV1_64"))
    set= MEMCACHED_HASH_FNV1_64;
  else if (!strcasecmp(opt_hash, "FNV1A_64"))
    set= MEMCACHED_HASH_FNV1A_64;
  else if (!strcasecmp(opt_hash, "FNV1_32"))
    set= MEMCACHED_HASH_FNV1_32;
  else if (!strcasecmp(opt_hash, "FNV1A_32"))
    set= MEMCACHED_HASH_FNV1A_32;
  else
  {
    fprintf(stderr, "hash: type not recognized %s\n", opt_hash);
    exit(1);
  }

  rc= memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_HASH, set);
  if (rc != MEMCACHED_SUCCESS)
  {
    fprintf(stderr, "hash: memcache error %s\n", memcached_strerror(memc, rc));
    exit(1);
  }
}

