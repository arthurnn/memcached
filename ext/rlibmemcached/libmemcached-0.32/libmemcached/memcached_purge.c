#include "common.h"
#include "memcached_io.h"
#include "memcached_constants.h"

memcached_return memcached_purge(memcached_server_st *ptr)
{
  uint32_t x;
  memcached_return ret= MEMCACHED_SUCCESS;

  if (ptr->root->purging || /* already purging */
      (memcached_server_response_count(ptr) < ptr->root->io_msg_watermark &&
      ptr->io_bytes_sent < ptr->root->io_bytes_watermark) ||
      (ptr->io_bytes_sent > ptr->root->io_bytes_watermark &&
      memcached_server_response_count(ptr) < 2))
  {
    return MEMCACHED_SUCCESS;
  }

  /* memcached_io_write and memcached_response may call memcached_purge
     so we need to be able stop any recursion.. */
  ptr->root->purging= 1;

  WATCHPOINT_ASSERT(ptr->fd != -1);
  /* Force a flush of the buffer to ensure that we don't have the n-1 pending
     requests buffered up.. */
  if (memcached_io_write(ptr, NULL, 0, 1) == -1)
  {
    ptr->root->purging= 0;
    return MEMCACHED_WRITE_FAILURE;
  }
  WATCHPOINT_ASSERT(ptr->fd != -1);

  uint32_t no_msg= memcached_server_response_count(ptr) - 1;
  if (no_msg > 0)
  {
    memcached_result_st result;
    memcached_result_st *result_ptr;
    char buffer[SMALL_STRING_LEN];

    /*
     * We need to increase the timeout, because we might be waiting for
     * data to be sent from the server (the commands was in the output buffer
     * and just flushed
     */
    int32_t timeo= ptr->root->poll_timeout;
    ptr->root->poll_timeout= 2000;

    result_ptr= memcached_result_create(ptr->root, &result);
    WATCHPOINT_ASSERT(result_ptr);

    for (x= 0; x < no_msg; x++)
    {
      memcached_result_reset(result_ptr);
      memcached_return rc= memcached_read_one_response(ptr, buffer,
                                                       sizeof (buffer),
                                                       result_ptr);
      /*
       * Purge doesn't care for what kind of command results that is received.
       * The only kind of errors I care about if is I'm out of sync with the
       * protocol or have problems reading data from the network..
       */
      if (rc== MEMCACHED_PROTOCOL_ERROR || rc == MEMCACHED_UNKNOWN_READ_FAILURE)
      {
        WATCHPOINT_ERROR(rc);
        ret = rc;
        memcached_io_reset(ptr);
      }
    }

    memcached_result_free(result_ptr);
    ptr->root->poll_timeout= timeo;
  }
  ptr->root->purging= 0;

  return ret;
}
