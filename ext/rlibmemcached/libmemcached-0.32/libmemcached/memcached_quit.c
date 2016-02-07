#include "common.h"

/*
  This closes all connections (forces flush of input as well).
  
  Maybe add a host specific, or key specific version? 
  
  The reason we send "quit" is that in case we have buffered IO, this 
  will force data to be completed.
*/

void memcached_quit_server(memcached_server_st *ptr, uint8_t io_death)
{
  if (ptr->fd != -1)
  {
    if (io_death == 0 && ptr->type != MEMCACHED_CONNECTION_UDP)
    {
      memcached_return rc;
      char buffer[MEMCACHED_MAX_BUFFER];

      if (ptr->root->flags & MEM_BINARY_PROTOCOL) 
      {
        protocol_binary_request_quit request = {.bytes= {0}};
        request.message.header.request.magic = PROTOCOL_BINARY_REQ;
        request.message.header.request.opcode = PROTOCOL_BINARY_CMD_QUIT;
        request.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
        rc= memcached_do(ptr, request.bytes, sizeof(request.bytes), 1);
      }
      else
        rc= memcached_do(ptr, "quit\r\n", 6, 1);

      WATCHPOINT_ASSERT(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_FETCH_NOTFINISHED);
      
      /* read until socket is closed, or there is an error
       * closing the socket before all data is read
       * results in server throwing away all data which is
       * not read
       */
      ssize_t nread;
      while (memcached_io_read(ptr, buffer, sizeof(buffer)/sizeof(*buffer),
                               &nread) == MEMCACHED_SUCCESS);
    }
    memcached_io_close(ptr);

    ptr->fd= -1;
    ptr->write_buffer_offset= (size_t) ((ptr->type == MEMCACHED_CONNECTION_UDP) ? UDP_DATAGRAM_HEADER_LENGTH : 0);
    ptr->read_buffer_length= 0;
    ptr->read_ptr= ptr->read_buffer;
    memcached_server_response_reset(ptr);
  }

  if (io_death)
  {
    ptr->server_failure_counter++;
  }
  else
  {
    ptr->server_failure_counter = 0;
  }
}

void memcached_quit(memcached_st *ptr)
{
  unsigned int x;

  if (ptr->hosts == NULL || 
      ptr->number_of_hosts == 0)
    return;

  if (ptr->hosts && ptr->number_of_hosts)
  {
    for (x= 0; x < ptr->number_of_hosts; x++)
      memcached_quit_server(&ptr->hosts[x], 0);
  }
}
