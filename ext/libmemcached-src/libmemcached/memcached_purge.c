#include <assert.h>

#include "common.h"
#include "memcached_io.h"

void memcached_purge(memcached_server_st *ptr) 
{
  int32_t timeout;
  char buffer[2048];
  size_t buffer_length= sizeof(buffer);
  memcached_result_st result;

  if (ptr->root->purging || /* already purging */
      (memcached_server_response_count(ptr) < ptr->root->io_msg_watermark && 
       ptr->io_bytes_sent < ptr->root->io_bytes_watermark) ||
      (ptr->io_bytes_sent > ptr->root->io_bytes_watermark && 
       memcached_server_response_count(ptr) < 10)) 
  {
    return;
  }

  /* memcached_io_write and memcached_response may call memcached_purge
     so we need to be able stop any recursion.. */
  ptr->root->purging= 1;

  /* Force a flush of the buffer to ensure that we don't have the n-1 pending
     requests buffered up.. */
  memcached_io_write(ptr, NULL, 0, 1);

  /* we have already incremented the response counter, and memcached_response
     will read out all messages.. To avoid memcached_response to wait forever
     for a response to a command I have in my buffer, let's decrement the 
     response counter :) */
  memcached_server_response_decrement(ptr);
  
  /* memcached_response may call memcached_io_read, but let's use a short
     timeout if there is no data yet */
  timeout= ptr->root->poll_timeout;
  ptr->root->poll_timeout= 1;
  memcached_response(ptr, buffer, sizeof(buffer), &result);
  ptr->root->poll_timeout= timeout;
  memcached_server_response_increment(ptr);
  ptr->root->purging = 0;
}
