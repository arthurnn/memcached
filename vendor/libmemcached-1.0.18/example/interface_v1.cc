/* -*- Mode: C; tab-width: 2; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/**
 * This file contains an implementation of the callback interface for level 1
 * in the protocol library. If you compare the implementation with the one
 * in interface_v0.cc you will see that this implementation is much easier and
 * hides all of the protocol logic and let you focus on the application
 * logic. One "problem" with this layer is that it is synchronous, so that
 * you will not receive the next command before a answer to the previous
 * command is being sent.
 */
#include "mem_config.h"

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <libmemcachedprotocol-0.0/handler.h>
#include <example/byteorder.h>
#include "example/memcached_light.h"
#include "example/storage.h"
#include "util/log.hpp"

static datadifferential::util::log_info_st *log_file= NULL;

static protocol_binary_response_status add_handler(const void *cookie,
                                                   const void *key,
                                                   uint16_t keylen,
                                                   const void *data,
                                                   uint32_t datalen,
                                                   uint32_t flags,
                                                   uint32_t exptime,
                                                   uint64_t *cas)
{
  (void)cookie;
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;
  struct item* item= get_item(key, keylen);
  if (item == NULL)
  {
    item= create_item(key, keylen, data, datalen, flags, (time_t)exptime);
    if (item == 0)
    {
      rval= PROTOCOL_BINARY_RESPONSE_ENOMEM;
    }
    else
    {
      put_item(item);
      *cas= item->cas;
      release_item(item);
    }
  }
  else
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS;
  }

  return rval;
}

static protocol_binary_response_status append_handler(const void *cookie,
                                                      const void *key,
                                                      uint16_t keylen,
                                                      const void* val,
                                                      uint32_t vallen,
                                                      uint64_t cas,
                                                      uint64_t *result_cas)
{
  (void)cookie;
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;

  struct item *item= get_item(key, keylen);
  struct item *nitem;

  if (item == NULL)
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_ENOENT;
  }
  else if (cas != 0 && cas != item->cas)
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS;
  }
  else if ((nitem= create_item(key, keylen, NULL, item->size + vallen,
                               item->flags, item->exp)) == NULL)
  {
    release_item(item);
    rval= PROTOCOL_BINARY_RESPONSE_ENOMEM;
  }
  else
  {
    memcpy(nitem->data, item->data, item->size);
    memcpy(((char*)(nitem->data)) + item->size, val, vallen);
    release_item(item);
    delete_item(key, keylen);
    put_item(nitem);
    *result_cas= nitem->cas;
    release_item(nitem);
  }

  return rval;
}

static protocol_binary_response_status decrement_handler(const void *cookie,
                                                         const void *key,
                                                         uint16_t keylen,
                                                         uint64_t delta,
                                                         uint64_t initial,
                                                         uint32_t expiration,
                                                         uint64_t *result,
                                                         uint64_t *result_cas) {
  (void)cookie;
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;
  uint64_t val= initial;
  struct item *item= get_item(key, keylen);

  if (item != NULL)
  {
    if (delta > *(uint64_t*)item->data)
      val= 0;
    else
      val= *(uint64_t*)item->data - delta;

    expiration= (uint32_t)item->exp;
    release_item(item);
    delete_item(key, keylen);
  }

  item= create_item(key, keylen, NULL, sizeof(initial), 0, (time_t)expiration);
  if (item == 0)
  {
    rval= PROTOCOL_BINARY_RESPONSE_ENOMEM;
  }
  else
  {
    memcpy(item->data, &val, sizeof(val));
    put_item(item);
    *result= val;
    *result_cas= item->cas;
    release_item(item);
  }

  return rval;
}

static protocol_binary_response_status delete_handler(const void *, // cookie
                                                      const void *key,
                                                      uint16_t keylen,
                                                      uint64_t cas)
{
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;

  if (cas != 0)
  {
    struct item *item= get_item(key, keylen);
    if (item != NULL)
    {
      if (item->cas != cas)
      {
        release_item(item);
        return PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS;
      }
      release_item(item);
    }
  }

  if (!delete_item(key, keylen))
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_ENOENT;
  }

  return rval;
}


static protocol_binary_response_status flush_handler(const void * /* cookie */, uint32_t /* when */)
{
  return PROTOCOL_BINARY_RESPONSE_SUCCESS;
}

static protocol_binary_response_status get_handler(const void *cookie,
                                                   const void *key,
                                                   uint16_t keylen,
                                                   memcached_binary_protocol_get_response_handler response_handler) {
  struct item *item= get_item(key, keylen);

  if (item == NULL)
  {
    return PROTOCOL_BINARY_RESPONSE_KEY_ENOENT;
  }

  protocol_binary_response_status rc;
  rc= response_handler(cookie, key, (uint16_t)keylen,
                          item->data, (uint32_t)item->size, item->flags,
                          item->cas);
  release_item(item);
  return rc;
}

static protocol_binary_response_status increment_handler(const void *cookie,
                                                         const void *key,
                                                         uint16_t keylen,
                                                         uint64_t delta,
                                                         uint64_t initial,
                                                         uint32_t expiration,
                                                         uint64_t *result,
                                                         uint64_t *result_cas) {
  (void)cookie;
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;
  uint64_t val= initial;
  struct item *item= get_item(key, keylen);

  if (item != NULL)
  {
    val= (*(uint64_t*)item->data) + delta;
    expiration= (uint32_t)item->exp;
    release_item(item);
    delete_item(key, keylen);
  }

  item= create_item(key, keylen, NULL, sizeof(initial), 0, (time_t)expiration);
  if (item == NULL)
  {
    rval= PROTOCOL_BINARY_RESPONSE_ENOMEM;
  }
  else
  {
    char buffer[1024] = {0};
    memcpy(buffer, key, keylen);
    memcpy(item->data, &val, sizeof(val));
    put_item(item);
    *result= val;
    *result_cas= item->cas;
    release_item(item);
  }

  return rval;
}

static protocol_binary_response_status noop_handler(const void *cookie) {
  (void)cookie;
  return PROTOCOL_BINARY_RESPONSE_SUCCESS;
}

static protocol_binary_response_status prepend_handler(const void *cookie,
                                                       const void *key,
                                                       uint16_t keylen,
                                                       const void* val,
                                                       uint32_t vallen,
                                                       uint64_t cas,
                                                       uint64_t *result_cas) {
  (void)cookie;
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;

  struct item *item= get_item(key, keylen);
  struct item *nitem= NULL;

  if (item == NULL)
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_ENOENT;
  }
  else if (cas != 0 && cas != item->cas)
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS;
  }
  else if ((nitem= create_item(key, keylen, NULL, item->size + vallen,
                                 item->flags, item->exp)) == NULL)
  {
    rval= PROTOCOL_BINARY_RESPONSE_ENOMEM;
  }
  else
  {
    memcpy(nitem->data, val, vallen);
    memcpy(((char*)(nitem->data)) + vallen, item->data, item->size);
    release_item(item);
    item= NULL;
    delete_item(key, keylen);
    put_item(nitem);
    *result_cas= nitem->cas;
  }

  if (item)
    release_item(item);

  if (nitem)
    release_item(nitem);

  return rval;
}

static protocol_binary_response_status quit_handler(const void *) //cookie
{
  return PROTOCOL_BINARY_RESPONSE_SUCCESS;
}

static protocol_binary_response_status replace_handler(const void *, // cookie
                                                       const void *key,
                                                       uint16_t keylen,
                                                       const void* data,
                                                       uint32_t datalen,
                                                       uint32_t flags,
                                                       uint32_t exptime,
                                                       uint64_t cas,
                                                       uint64_t *result_cas)
{
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;
  struct item* item= get_item(key, keylen);

  if (item == NULL)
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_ENOENT;
  }
  else if (cas == 0 || cas == item->cas)
  {
    release_item(item);
    delete_item(key, keylen);
    item= create_item(key, keylen, data, datalen, flags, (time_t)exptime);
    if (item == 0)
    {
      rval= PROTOCOL_BINARY_RESPONSE_ENOMEM;
    }
    else
    {
      put_item(item);
      *result_cas= item->cas;
      release_item(item);
    }
  }
  else
  {
    rval= PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS;
    release_item(item);
  }

  return rval;
}

static protocol_binary_response_status set_handler(const void *cookie,
                                                   const void *key,
                                                   uint16_t keylen,
                                                   const void* data,
                                                   uint32_t datalen,
                                                   uint32_t flags,
                                                   uint32_t exptime,
                                                   uint64_t cas,
                                                   uint64_t *result_cas) {
  (void)cookie;
  protocol_binary_response_status rval= PROTOCOL_BINARY_RESPONSE_SUCCESS;

  if (cas != 0)
  {
    struct item* item= get_item(key, keylen);
    if (item != NULL && cas != item->cas)
    {
      /* Invalid CAS value */
      release_item(item);
      return PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS;
    }
  }

  delete_item(key, keylen);
  struct item* item= create_item(key, keylen, data, datalen, flags, (time_t)exptime);
  if (item == 0)
  {
    rval= PROTOCOL_BINARY_RESPONSE_ENOMEM;
  }
  else
  {
    put_item(item);
    *result_cas= item->cas;
    release_item(item);
  }

  return rval;
}

static protocol_binary_response_status stat_handler(const void *cookie,
                                                    const void *, // key
                                                    uint16_t, // keylen,
                                                    memcached_binary_protocol_stat_response_handler response_handler)
{
  /* Just return an empty packet */
  return response_handler(cookie, NULL, 0, NULL, 0);
}

static protocol_binary_response_status version_handler(const void *cookie,
                                                       memcached_binary_protocol_version_response_handler response_handler)
{
  const char *version= "0.1.1";
  return response_handler(cookie, version, (uint32_t)strlen(version));
}

memcached_binary_protocol_callback_st interface_v1_impl;

void initialize_interface_v1_handler(datadifferential::util::log_info_st& arg)
{
  log_file= &arg;
  memset(&interface_v1_impl, 0, sizeof(memcached_binary_protocol_callback_st));

  interface_v1_impl.interface_version= MEMCACHED_PROTOCOL_HANDLER_V1;
  interface_v1_impl.interface.v1.add= add_handler;
  interface_v1_impl.interface.v1.append= append_handler;
  interface_v1_impl.interface.v1.decrement= decrement_handler;
  interface_v1_impl.interface.v1.delete_object= delete_handler;
  interface_v1_impl.interface.v1.flush_object= flush_handler;
  interface_v1_impl.interface.v1.get= get_handler;
  interface_v1_impl.interface.v1.increment= increment_handler;
  interface_v1_impl.interface.v1.noop= noop_handler;
  interface_v1_impl.interface.v1.prepend= prepend_handler;
  interface_v1_impl.interface.v1.quit= quit_handler;
  interface_v1_impl.interface.v1.replace= replace_handler;
  interface_v1_impl.interface.v1.set= set_handler;
  interface_v1_impl.interface.v1.stat= stat_handler;
  interface_v1_impl.interface.v1.version= version_handler;
}
