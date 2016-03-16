/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2012-2013 Data Differential, http://datadifferential.com/
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


#if defined(HAVE_UUID_UUID_H) && HAVE_UUID_UUID_H
#include <uuid/uuid.h>
#endif

struct keys_st {
public:
  keys_st(size_t arg)
  {
    init(arg, UUID_STRING_MAXLENGTH);
  }

  keys_st(size_t arg, size_t padding)
  {
    init(arg, padding);
  }

  void init(size_t arg, size_t padding)
  {
    _lengths.resize(arg);
    _keys.resize(arg);

    for (size_t x= 0; x < _keys.size(); x++)
    {
      libtest::vchar_t key_buffer;
      key_buffer.resize(padding +1);
      memset(&key_buffer[0], 'x', padding);

#if defined(HAVE_UUID_UUID_H) && HAVE_UUID_UUID_H
      if (HAVE_UUID_UUID_H)
      {
        uuid_t out;
        uuid_generate(out);

        uuid_unparse(out, &key_buffer[0]);
        _keys[x]= strdup(&key_buffer[0]);
        (_keys[x])[UUID_STRING_MAXLENGTH]= 'x';
      }
      else // We just use a number and pad the string if UUID is not available
#endif
      {
        char int_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
        int key_length= snprintf(int_buffer, sizeof(int_buffer), "%u", uint32_t(x));
        memcpy(&key_buffer[0], int_buffer, key_length);
        _keys[x]= strdup(&key_buffer[0]);
      }
      _lengths[x]= padding;
    }
  }

  ~keys_st()
  {
    for (libtest::vchar_ptr_t::iterator iter= _keys.begin();
         iter != _keys.end();
         ++iter)
    {
      ::free(*iter);
    }
  }

  libtest::vchar_ptr_t::iterator begin()
  {
    return _keys.begin();
  }

  libtest::vchar_ptr_t::iterator end()
  {
    return _keys.end();
  }

  size_t size() const
  {
    return _keys.size();
  }

  std::vector<size_t>& lengths()
  {
    return _lengths;
  }

  libtest::vchar_ptr_t& keys()
  {
    return _keys;
  }

  size_t* lengths_ptr()
  {
    return &_lengths[0];
  }

  char** keys_ptr()
  {
    return &_keys[0];
  }

  char* key_at(size_t arg)
  {
    return _keys[arg];
  }

  size_t length_at(size_t arg)
  {
    return _lengths[arg];
  }

private:
    libtest::vchar_ptr_t _keys;
    std::vector<size_t> _lengths;
};
