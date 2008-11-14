#include "common.h"

memcached_return memcached_do(memcached_server_st *ptr, const void *command, 
                              size_t command_length, uint8_t with_flush)
{
  memcached_return rc;
  ssize_t sent_length;

  WATCHPOINT_ASSERT(command_length);
  WATCHPOINT_ASSERT(command);

  if ((rc= memcached_connect(ptr)) != MEMCACHED_SUCCESS)
    return rc;

  sent_length= memcached_io_write(ptr, command, command_length, with_flush);

  if (sent_length == -1 || (size_t)sent_length != command_length)
    rc= MEMCACHED_WRITE_FAILURE;
  else
    memcached_server_response_increment(ptr);

  return rc;
}
