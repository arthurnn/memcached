/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "mem_config.h"
#include <assert.h>

#include <libmemcachedprotocol-0.0/handler.h>
#include <libmemcachedprotocol/cache.h>
#include <libmemcached/byteorder.h>
#include <libmemcached/socket.hpp>

/*
 * I don't really need the following two functions as function pointers
 * in the instance handle, but I don't want to put them in the global
 * namespace for those linking statically (personally I don't like that,
 * but some people still do). If it ever shows up as a performance thing
 * I'll look into optimizing this ;-)
 */
typedef bool (*drain_func)(memcached_protocol_client_st *client);
typedef protocol_binary_response_status (*spool_func)(memcached_protocol_client_st *client,
                                                      const void *data,
                                                      size_t length);

/**
 * Definition of the per instance structure.
 */
struct memcached_protocol_st {
  memcached_binary_protocol_callback_st *callback;
  memcached_protocol_recv_func recv;
  memcached_protocol_send_func send;

  /*
   * I really don't need these as funciton pointers, but I don't want
   * to clutter the namespace if someone links statically.
   */
  drain_func drain;
  spool_func spool;

  /*
   * To avoid keeping a buffer in each client all the time I have a
   * bigger buffer in the instance that I read to initially, and then
   * I try to parse and execute as much from the buffer. If I wasn't able
   * to process all data I'll keep that in a per-connection buffer until
   * the next time I can read from the socket.
   */
  uint8_t *input_buffer;
  size_t input_buffer_size;

  bool pedantic;
  /* @todo use multiple sized buffers */
  cache_t *buffer_cache;
};

struct chunk_st {
  /* Pointer to the data */
  char *data;
  /* The offset to the first byte into the buffer that is used */
  size_t offset;
  /* The offset into the buffer for the first free byte */
  size_t nbytes;
  /* The number of bytes in the buffer */
  size_t size;
  /* Pointer to the next buffer in the chain */
  struct chunk_st *next;
};

#define CHUNK_BUFFERSIZE 2048

typedef memcached_protocol_event_t (*process_data)(struct memcached_protocol_client_st *client, ssize_t *length, void **endptr);

enum ascii_cmd {
  GET_CMD,
  GETS_CMD,
  SET_CMD,
  ADD_CMD,
  REPLACE_CMD,
  CAS_CMD,
  APPEND_CMD,
  PREPEND_CMD,
  DELETE_CMD,
  INCR_CMD,
  DECR_CMD,
  STATS_CMD,
  FLUSH_ALL_CMD,
  VERSION_CMD,
  QUIT_CMD,
  VERBOSITY_CMD,
  UNKNOWN_CMD
};

struct memcached_protocol_client_st {
  bool is_verbose;
  memcached_protocol_st *root;
  memcached_socket_t sock;
  int error;

  /* Linked list of data to send */
  struct chunk_st *output;
  struct chunk_st *output_tail;

  /*
   * While we process input data, this is where we spool incomplete commands
   * if we need to receive more data....
   * @todo use the buffercace
   */
  uint8_t *input_buffer;
  size_t input_buffer_size;
  size_t input_buffer_offset;

  /* The callback to the protocol handler to use (ascii or binary) */
  process_data work;

  /*
   * Should the spool data discard the data to send or not? (aka noreply in
   * the ascii protocol..
   */
  bool mute;

  /* Members used by the binary protocol */
  protocol_binary_request_header *current_command;

  /* Members used by the ascii protocol */
  enum ascii_cmd ascii_command;
};

#include "ascii_handler.h"
#include "binary_handler.h"
