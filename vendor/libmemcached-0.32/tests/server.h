/*
  Server startup and shutdown functions.
*/
#ifdef	__cplusplus
extern "C" {
#endif

#include <libmemcached/memcached.h>

typedef struct server_startup_st server_startup_st;

struct server_startup_st
{
  uint8_t count;
  uint8_t udp;
  memcached_server_st *servers;
  char *server_list;
};

void server_startup(server_startup_st *construct);
void server_shutdown(server_startup_st *construct);

#ifdef	__cplusplus
}
#endif
