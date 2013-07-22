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

  sent_length= memcached_io_write(ptr, command, command_length, (char) with_flush);

  if (sent_length == -1 || (size_t)sent_length != command_length)
    rc= MEMCACHED_WRITE_FAILURE;
  else if ((ptr->root->flags & MEM_NOREPLY) == 0)
    memcached_server_response_increment(ptr);

  return rc;
}

memcached_return memcached_vdo(memcached_server_st *ptr,
                               const struct libmemcached_io_vector_st *vector, size_t count,
                               uint8_t with_flush)
{
  memcached_return rc;
  ssize_t sent_length;
	size_t command_length;
  uint32_t x;

  WATCHPOINT_ASSERT(count);
  WATCHPOINT_ASSERT(vector);

  if ((rc= memcached_connect(ptr)) != MEMCACHED_SUCCESS)
  {
    WATCHPOINT_ERROR(rc);
    return rc;
  }

  if (ptr->type == MEMCACHED_CONNECTION_UDP && with_flush && ptr->write_buffer_offset > UDP_DATAGRAM_HEADER_LENGTH)
  {
    memcached_io_write(ptr, NULL, 0, true);
  }

  sent_length= memcached_io_writev(ptr, vector, count, (char) with_flush);

  command_length = 0;
  for (x= 0; x < count; ++x, vector++)
    command_length+= vector->length;

  if (sent_length == -1 || (size_t)sent_length != command_length) {
    rc = MEMCACHED_WRITE_FAILURE;
    WATCHPOINT_ERROR(rc);
  } else if ((ptr->root->flags & MEM_NOREPLY) == 0)
    memcached_server_response_increment(ptr);

  return rc;
}
