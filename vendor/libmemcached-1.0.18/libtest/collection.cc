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

#include "libtest/yatlcon.h"

#include <libtest/common.h>

// @todo possibly have this code fork off so if it fails nothing goes bad
static test_return_t runner_code(libtest::Framework* frame,
                                 test_st* run, 
                                 libtest::Timer& _timer)
{ // Runner Code

  assert(frame->runner());
  assert(run->test_fn);

  test_return_t return_code;
  try 
  {
    _timer.reset();
    assert(frame);
    assert(frame->runner());
    assert(run->test_fn);
    return_code= frame->runner()->main(run->test_fn, frame->creators_ptr());
  }
  // Special case where check for the testing of the exception
  // system.
  catch (const libtest::fatal& e)
  {
    if (libtest::fatal::is_disabled())
    {
      libtest::fatal::increment_disabled_counter();
      return_code= TEST_SUCCESS;
    }
    else
    {
      throw;
    }
  }

  _timer.sample();

  return return_code;
}

namespace libtest {

Collection::Collection(Framework* frame_arg,
                       collection_st* arg) :
  _name(arg->name),
  _pre(arg->pre),
  _post(arg->post),
  _tests(arg->tests),
  _frame(frame_arg),
  _success(0),
  _skipped(0),
  _failed(0),
  _total(0),
  _formatter(frame_arg->name(), _name)
{
  fatal_assert(arg);
}

test_return_t Collection::exec()
{
  if (test_success(_frame->runner()->setup(_pre, _frame->creators_ptr())))
  {
    for (test_st *run= _tests; run->name; run++)
    {
      formatter()->push_testcase(run->name);
      if (_frame->match(run->name))
      {
        formatter()->skipped();
        continue;
      }
      _total++;

      test_return_t return_code;
      try 
      {
        if (run->requires_flush)
        {
          if (test_failed(_frame->runner()->flush(_frame->creators_ptr())))
          {
            Error << "frame->runner()->flush(creators_ptr)";
            _skipped++;
            formatter()->skipped();
            continue;
          }
        }

        set_alarm();

        try 
        {
          return_code= runner_code(_frame, run, _timer);
        }
        catch (...)
        {
          cancel_alarm();

          throw;
        }
        libtest::cancel_alarm();
      }
      catch (const libtest::fatal& e)
      {
        stream::cerr(e.file(), e.line(), e.func()) << e.what();
        _failed++;
        formatter()->failed();
        throw;
      }

      switch (return_code)
      {
      case TEST_SUCCESS:
        _success++;
        formatter()->success(_timer);
        break;

      case TEST_FAILURE:
        _failed++;
        formatter()->failed();
        break;

      case TEST_SKIPPED:
        _skipped++;
        formatter()->skipped();
        break;

      default:
        FATAL("invalid return code");
      }
#if 0
      @TODO add code here to allow for a collection to define a method to reset to allow tests to continue.
#endif
    }

    (void) _frame->runner()->teardown(_post, _frame->creators_ptr());
  }

  if (_failed == 0 and _skipped == 0 and _success)
  {
    return TEST_SUCCESS;
  }

  if (_failed)
  {
    return TEST_FAILURE;
  }

  fatal_assert(_skipped or _success == 0);

  return TEST_SKIPPED;
}

} // namespace libtest

