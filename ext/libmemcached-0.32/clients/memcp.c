#include "libmemcached/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <string.h>

#include <libmemcached/memcached.h>

#include "client_options.h"
#include "utilities.h"

#define PROGRAM_NAME "memcp"
#define PROGRAM_DESCRIPTION "Copy a set of files to a memcached cluster."

/* Prototypes */
void options_parse(int argc, char *argv[]);

static int opt_binary=0;
static int opt_verbose= 0;
static char *opt_servers= NULL;
static char *opt_hash= NULL;
static int opt_method= OPT_SET;
static uint32_t opt_flags= 0;
static time_t opt_expires= 0;
static char *opt_username;
static char *opt_passwd;

int main(int argc, char *argv[])
{
  memcached_st *memc;
  memcached_return rc;
  memcached_server_st *servers;

  options_parse(argc, argv);

  memc= memcached_create(NULL);
  process_hash_option(memc, opt_hash);

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

  if (opt_servers)
    servers= memcached_servers_parse(opt_servers);
  else
    servers= memcached_servers_parse(argv[--argc]);

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
    struct stat sbuf;
    int fd;
    char *ptr;
    ssize_t read_length;
    char *file_buffer_ptr;

    fd= open(argv[optind], O_RDONLY);
    if (fd < 0)
    {
      fprintf(stderr, "memcp: %s: %s\n", argv[optind], strerror(errno));
      optind++;
      continue;
    }

    (void)fstat(fd, &sbuf);

    ptr= rindex(argv[optind], '/');
    if (ptr)
      ptr++;
    else
      ptr= argv[optind];

    if (opt_verbose) 
    {
      static const char *opstr[] = { "set", "add", "replace" };
      printf("op: %s\nsource file: %s\nlength: %zu\n"
	     "key: %s\nflags: %x\nexpires: %llu\n",
	     opstr[opt_method - OPT_SET], argv[optind], (size_t)sbuf.st_size,
	     ptr, opt_flags, (unsigned long long)opt_expires);
    }

    if ((file_buffer_ptr= (char *)malloc(sizeof(char) * (size_t)sbuf.st_size)) == NULL)
    {
      fprintf(stderr, "malloc: %s\n", strerror(errno)); 
      exit(1);
    }

    if ((read_length= read(fd, file_buffer_ptr, (size_t)sbuf.st_size)) == -1)
    {
      fprintf(stderr, "read: %s\n", strerror(errno)); 
      exit(1);
    }

    if (read_length != sbuf.st_size)
    {
      fprintf(stderr, "Failure reading from file\n");
      exit(1);
    }

    if (opt_method == OPT_ADD)
      rc= memcached_add(memc, ptr, strlen(ptr),
                        file_buffer_ptr, (size_t)sbuf.st_size,
			opt_expires, opt_flags);
    else if (opt_method == OPT_REPLACE)
      rc= memcached_replace(memc, ptr, strlen(ptr),
			    file_buffer_ptr, (size_t)sbuf.st_size,
			    opt_expires, opt_flags);
    else
      rc= memcached_set(memc, ptr, strlen(ptr),
                        file_buffer_ptr, (size_t)sbuf.st_size,
                        opt_expires, opt_flags);

    if (rc != MEMCACHED_SUCCESS)
    {
      fprintf(stderr, "memcp: %s: memcache error %s", 
	      ptr, memcached_strerror(memc, rc));
      if (memc->cached_errno)
	fprintf(stderr, " system error %s", strerror(memc->cached_errno));
      fprintf(stderr, "\n");
    }

    free(file_buffer_ptr);
    close(fd);
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
    case OPT_FLAG: /* --flag */
      opt_flags= (uint32_t)strtol(optarg, (char **)NULL, 16);
      break;
    case OPT_EXPIRE: /* --expire */
      opt_expires= (time_t)strtoll(optarg, (char **)NULL, 10);
      break;
    case OPT_SET:
      opt_method= OPT_SET;
      break;
    case OPT_REPLACE:
      opt_method= OPT_REPLACE;
      break;
    case OPT_ADD:
      opt_method= OPT_ADD;
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
