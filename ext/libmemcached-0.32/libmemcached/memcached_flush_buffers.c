#include "common.h"
#include "memcached_io.h"

memcached_return memcached_flush_buffers(memcached_st *mem)
{
  memcached_return ret= MEMCACHED_SUCCESS;
  uint32_t x;

  for (x= 0; x < mem->number_of_hosts; ++x)
    if (mem->hosts[x].write_buffer_offset != 0) 
    {
      if (mem->hosts[x].fd == -1 &&
          (ret= memcached_connect(&mem->hosts[x])) != MEMCACHED_SUCCESS)
      {
        WATCHPOINT_ERROR(ret);
        return ret;
      }
      if (memcached_io_write(&mem->hosts[x], NULL, 0, 1) == -1)
        ret= MEMCACHED_SOME_ERRORS;
    }

  return ret;
}
