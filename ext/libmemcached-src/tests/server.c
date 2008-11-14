/*
  Startup, and shutdown the memcached servers.
*/

#define TEST_PORT_BASE MEMCACHED_DEFAULT_PORT+10 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <libmemcached/memcached.h>
#include <unistd.h>
#include "server.h"

void server_startup(server_startup_st *construct)
{
  unsigned int x;

  if ((construct->server_list= getenv("MEMCACHED_SERVERS")))
  {
    printf("servers %s\n", construct->server_list);
    construct->servers= memcached_servers_parse(construct->server_list);
    construct->server_list= NULL;
    construct->count= 0;
  }
  else
  {
    {
      char server_string_buffer[8096];
      char *end_ptr;
      end_ptr= server_string_buffer;

      for (x= 0; x < construct->count; x++)
      {
        char buffer[1024]; /* Nothing special for number */
        int count;
        int status;

        if (construct->udp){
          if(x == 0) {
            sprintf(buffer, "memcached -d -P /tmp/%umemc.pid -t 1 -U %u -m 128", x, x+ TEST_PORT_BASE);
          } else {
            sprintf(buffer, "memcached -d -P /tmp/%umemc.pid -t 1 -U %u", x, x+ TEST_PORT_BASE);
          }
        }
        else{
          if(x == 0) {
            sprintf(buffer, "memcached -d -P /tmp/%umemc.pid -t 1 -p %u -m 128", x, x+ TEST_PORT_BASE);
          } else {
            sprintf(buffer, "memcached -d -P /tmp/%umemc.pid -t 1 -p %u", x, x+ TEST_PORT_BASE);
          }
        }
        status= system(buffer);
        count= sprintf(end_ptr, "localhost:%u,", x + TEST_PORT_BASE);
        end_ptr+= count;
      }
      *end_ptr= 0;

      construct->server_list= strdup(server_string_buffer);
    }
    printf("servers %s\n", construct->server_list);
    construct->servers= memcached_servers_parse(construct->server_list);
  }

  assert(construct->servers);

  srandom(time(NULL));

  for (x= 0; x < memcached_server_list_count(construct->servers); x++)
  {
    printf("\t%s : %u\n", construct->servers[x].hostname, construct->servers[x].port);
    assert(construct->servers[x].fd == -1);
    assert(construct->servers[x].cursor_active == 0);
  }

  printf("\n");
}

void server_shutdown(server_startup_st *construct)
{
  unsigned int x;

  if (construct->server_list)
  {
    for (x= 0; x < construct->count; x++)
    {
      char buffer[1024]; /* Nothing special for number */
      sprintf(buffer, "cat /tmp/%umemc.pid | xargs kill", x);
      system(buffer);

      sprintf(buffer, "/tmp/%umemc.pid", x);
      unlink(buffer);
    }

    free(construct->server_list);
  }
}
