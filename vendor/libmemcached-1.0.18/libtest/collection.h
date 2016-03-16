/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Data Differential YATL (i.e. libtest)  library
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
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

#include <libtest/formatter.hpp>

#include <libtest/timer.hpp>

namespace { class Framework; }


/**
  A structure which describes a collection of test cases.
*/
struct collection_st {
  const char *name;
  test_callback_fn *pre;
  test_callback_fn *post;
  struct test_st *tests;
};

namespace libtest {

class Collection {
public:
  Collection(libtest::Framework*, collection_st*);

  test_return_t exec();

  const char* name()
  {
    return _name.c_str();
  }

  uint32_t success()
  {
    return _success;
  }

  uint32_t skipped()
  {
    return _skipped;
  }

  uint32_t failed()
  {
    return _failed;
  }

  uint32_t total()
  {
    return _total;
  }

  libtest::Formatter* formatter()
  {
    return &_formatter;
  }

private:
  std::string _name;
  test_callback_fn *_pre;
  test_callback_fn *_post;
  struct test_st *_tests;
  libtest::Framework* _frame;
  uint32_t _success;
  uint32_t _skipped;
  uint32_t _failed;
  uint32_t _total;
  libtest::Timer _timer;
  libtest::Formatter _formatter;

private:
  Collection( const Collection& );
  const Collection& operator=( const Collection& );
};

} // namespace libtest
