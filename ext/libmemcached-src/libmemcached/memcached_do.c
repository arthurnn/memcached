#include "common.h"

memcached_return memcached_do(memcached_server_st *ptr, const void *command, 
                              size_t command_length, uint8_t with_flush)
{
  memcached_return rc;
  ssize_t sent_length;

  WATCHPOINT_ASSERT(command_length);
  WATCHPOINT_ASSERT(command);

  if ((rc= memcached_connect(ptr)) != MEMCACHED_SUCCESS)
  {
    WATCHPOINT_ERROR(rc);
    return rc;
  }

  /*
  ** Since non buffering ops in UDP mode dont check to make sure they will fit
  ** before they start writing, if there is any data in buffer, clear it out,
  ** otherwise we might get a partial write.
  **/
  if (ptr->type == MEMCACHED_CONNECTION_UDP && with_flush && ptr->write_buffer_offset > UDP_DATAGRAM_HEADER_LENGTH)
    memcached_io_write(ptr, NULL, 0, 1);

  sent_length= memcached_io_write(ptr, command, command_length, with_flush);

  if (sent_length == -1 || (size_t)sent_length != command_length)
    rc= MEMCACHED_WRITE_FAILURE;
  else if ((ptr->root->flags & MEM_NOREPLY) == 0)
    memcached_server_response_increment(ptr);

  return rc;
}
