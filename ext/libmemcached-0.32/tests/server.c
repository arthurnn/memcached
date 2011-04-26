/*
  Startup, and shutdown the memcached servers.
*/

#define TEST_PORT_BASE MEMCACHED_DEFAULT_PORT+10 

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
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

        sprintf(buffer, "/tmp/%umemc.pid", x);
        if (access(buffer, F_OK) == 0) 
        {
          FILE *fp= fopen(buffer, "r");
          remove(buffer);

          if (fp != NULL)
          {
            if (fgets(buffer, sizeof(buffer), fp) != NULL)
            { 
              pid_t pid = (pid_t)atol(buffer);
              if (pid != 0) 
                kill(pid, SIGTERM);
            }

            fclose(fp);
          }
        }

        if (x == 0)
        {
          sprintf(buffer, "%s -d -P /tmp/%umemc.pid -t 1 -p %u -U %u -m 128",
                    MEMCACHED_BINARY, x, x + TEST_PORT_BASE, x + TEST_PORT_BASE);
        } 
        else
        {
          sprintf(buffer, "%s -d -P /tmp/%umemc.pid -t 1 -p %u -U %u",
                    MEMCACHED_BINARY, x, x + TEST_PORT_BASE, x + TEST_PORT_BASE);
        }
        fprintf(stderr, "STARTING SERVER: %s\n", buffer);
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

  srandom((unsigned int)time(NULL));

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
      /* We have to check the return value of this or the compiler will yell */
      int sys_ret= system(buffer);
      assert(sys_ret != -1); 
      sprintf(buffer, "/tmp/%umemc.pid", x);
      unlink(buffer);
    }

    free(construct->server_list);
  }
}
