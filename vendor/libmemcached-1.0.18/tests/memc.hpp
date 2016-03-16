/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
 *  All rights reserved.
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

namespace test {

class Memc {
public:
  Memc()
  {
    _memc= memcached_create(NULL);

    if (_memc == NULL)
    {
      throw "memcached_create() failed";
    }
  }

  Memc(const memcached_st* arg)
  {
    _memc= memcached_clone(NULL, arg);

    if (_memc == NULL)
    {
      throw "memcached_clone() failed";
    }
  }

  Memc(const std::string& arg)
  {
    _memc= memcached(arg.c_str(), arg.size());
    if (_memc == NULL)
    {
      throw "memcached() failed";
    }
  }

  Memc(in_port_t arg)
  {
    _memc= memcached_create(NULL);

    if (_memc == NULL)
    {
      throw "memcached_create() failed";
    }
    memcached_server_add(_memc, "localhost", arg);
  }

  memcached_st* operator&() const
  { 
    return _memc;
  }

  memcached_st* operator->() const
  { 
    return _memc;
  }

  ~Memc()
  {
    memcached_free(_memc);
  }

private:
  memcached_st *_memc;

};

} // namespace test
