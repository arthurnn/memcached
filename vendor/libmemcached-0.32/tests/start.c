#include <stdio.h>
#include <string.h>
#include "server.h"

int main(void)
{
  server_startup_st construct;

  memset(&construct, 0, sizeof(server_startup_st));

  construct.count= 4;

  server_startup(&construct);

  return 0;
}
