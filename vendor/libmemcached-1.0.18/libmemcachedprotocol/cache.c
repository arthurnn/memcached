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

/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include "mem_config.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifndef NDEBUG
#include <signal.h>
#endif

#include <libmemcachedprotocol/common.h>

#ifndef NDEBUG
const uint64_t redzone_pattern = 0xdeadbeefcafebabe;
int cache_error = 0;
#endif

const size_t initial_pool_size = 64;

cache_t* cache_create(const char *name, size_t bufsize, size_t align,
                      cache_constructor_t* constructor,
                      cache_destructor_t* destructor) {
    cache_t* ret = calloc(1, sizeof(cache_t));
    size_t name_length= strlen(name);
    char* nm= calloc(1, (sizeof(char) * name_length) +1);
    memcpy(nm, name, name_length);
    void** ptr = calloc(initial_pool_size, bufsize);
    if (ret == NULL || nm == NULL || ptr == NULL ||
        pthread_mutex_init(&ret->mutex, NULL) == -1) {
        free(ret);
        free(nm);
        free(ptr);
        return NULL;
    }

    ret->name = nm;
    ret->ptr = ptr;
    ret->freetotal = initial_pool_size;
    ret->constructor = constructor;
    ret->destructor = destructor;

#ifndef NDEBUG
    ret->bufsize = bufsize + 2 * sizeof(redzone_pattern);
#else
    ret->bufsize = bufsize;
#endif

    (void)align;

    return ret;
}

static inline void* get_object(void *ptr) {
#ifndef NDEBUG
    uint64_t *pre = ptr;
    return pre + 1;
#else
    return ptr;
#endif
}

void cache_destroy(cache_t *cache) {
    while (cache->freecurr > 0) {
        void *ptr = cache->ptr[--cache->freecurr];
        if (cache->destructor) {
            cache->destructor(get_object(ptr), NULL);
        }
        free(ptr);
    }
    free(cache->name);
    free(cache->ptr);
    pthread_mutex_destroy(&cache->mutex);
}

void* cache_alloc(cache_t *cache) {
    void *ret;
    void *object;
    pthread_mutex_lock(&cache->mutex);
    if (cache->freecurr > 0) {
        ret = cache->ptr[--cache->freecurr];
        object = get_object(ret);
    } else {
        object = ret = malloc(cache->bufsize);
        if (ret != NULL) {
            object = get_object(ret);

            if (cache->constructor != NULL &&
                cache->constructor(object, NULL, 0) != 0) {
                free(ret);
                object = NULL;
            }
        }
    }
    pthread_mutex_unlock(&cache->mutex);

#ifndef NDEBUG
    if (object != NULL) {
        /* add a simple form of buffer-check */
        uint64_t *pre = ret;
        *pre = redzone_pattern;
        ret = pre+1;
        memcpy(((char*)ret) + cache->bufsize - (2 * sizeof(redzone_pattern)),
               &redzone_pattern, sizeof(redzone_pattern));
    }
#endif

    return object;
}

void cache_free(cache_t *cache, void *ptr) {
    pthread_mutex_lock(&cache->mutex);

#ifndef NDEBUG
    /* validate redzone... */
    if (memcmp(((char*)ptr) + cache->bufsize - (2 * sizeof(redzone_pattern)),
               &redzone_pattern, sizeof(redzone_pattern)) != 0) {
        raise(SIGABRT);
        cache_error = 1;
        pthread_mutex_unlock(&cache->mutex);
        return;
    }
    uint64_t *pre = ptr;
    --pre;
    if (*pre != redzone_pattern) {
        raise(SIGABRT);
        cache_error = -1;
        pthread_mutex_unlock(&cache->mutex);
        return;
    }
    ptr = pre;
#endif
    if (cache->freecurr < cache->freetotal) {
        cache->ptr[cache->freecurr++] = ptr;
    } else {
        /* try to enlarge free connections array */
        size_t newtotal = cache->freetotal * 2;
        void **new_free = realloc(cache->ptr, sizeof(char *) * newtotal);
        if (new_free) {
            cache->freetotal = newtotal;
            cache->ptr = new_free;
            cache->ptr[cache->freecurr++] = ptr;
        } else {
            if (cache->destructor) {
                cache->destructor(ptr, NULL);
            }
            free(ptr);

        }
    }
    pthread_mutex_unlock(&cache->mutex);
}

