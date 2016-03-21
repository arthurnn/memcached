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
#include <libtest/collection.h>
#include <libtest/signal.h>

#include <algorithm>
#include <fnmatch.h>
#include <iostream>

namespace libtest {

Framework::Framework(libtest::SignalThread& signal_,
                     const std::string& name_,
                     const std::string& only_run_arg,
                     const std::string& wildcard_arg) :
  _total(0),
  _success(0),
  _skipped(0),
  _failed(0),
  _create(NULL),
  _destroy(NULL),
  _on_error(NULL),
  _runner(NULL),
  _socket(false),
  _creators_ptr(NULL),
  _signal(signal_),
  _only_run(only_run_arg),
  _wildcard(wildcard_arg),
  _name(name_)
{
  get_world(this);
}

void Framework::collections(collection_st collections_[])
{
  for (collection_st *next= collections_; next and next->name; next++)
  {
    _collection.push_back(new Collection(this, next));
  }
}

Framework::~Framework()
{
  if (_destroy and _destroy(_creators_ptr))
  {
    Error << "Failure in _destroy(), some resources may not have been cleaned up.";
  }

  _servers.shutdown();

  delete _runner;

  std::for_each(_collection.begin(), _collection.end(), DeleteFromVector());
  _collection.clear();
}

bool Framework::match(const char* arg)
{
  if (_wildcard.empty() == false and fnmatch(_wildcard.c_str(), arg, 0))
  {
    return true;
  }

  return false;
}

void Framework::exec()
{
  for (std::vector<Collection*>::iterator iter= _collection.begin();
       iter != _collection.end() and (_signal.is_shutdown() == false);
       ++iter)
  {
    if (*iter)
    {
      if (_only_run.empty() == false and
          fnmatch(_only_run.c_str(), (*iter)->name(), 0))
      {
        continue;
      }

      _total++;

      try {
        switch ((*iter)->exec())
        {
          case TEST_FAILURE:
            _failed++;
            break;

          case TEST_SKIPPED:
            _skipped++;
            break;

            // exec() can return SUCCESS, but that doesn't mean that some tests did
            // not fail or get skipped.
          case TEST_SUCCESS:
            _success++;
            break;
        }
      }
      catch (const libtest::fatal& e)
      {
        _failed++;
        stream::cerr(e.file(), e.line(), e.func()) << e.what();
      }
      catch (const libtest::disconnected& e)
      {
        _failed++;
        Error << "Unhandled disconnection occurred:" << e.what();
        throw;
      }
      catch (...)
      {
        _failed++;
        throw;
      }
    }
  }

  void xml(const std::string& testsuites_name, std::ostream& output);
}

uint32_t Framework::sum_total()
{
  uint32_t count= 0;
  for (std::vector<Collection*>::iterator iter= _collection.begin();
       iter != _collection.end();
       ++iter)
  {
    count+= (*iter)->total();
  }

  return count;
}

uint32_t Framework::sum_success()
{
  uint32_t count= 0;
  for (std::vector<Collection*>::iterator iter= _collection.begin();
       iter != _collection.end();
       ++iter)
  {
    count+= (*iter)->success();
  }

  return count;
}

uint32_t Framework::sum_skipped()
{
  uint32_t count= 0;
  for (std::vector<Collection*>::iterator iter= _collection.begin();
       iter != _collection.end();
       ++iter)
  {
    count+= (*iter)->skipped();
  }

  return count;
}

uint32_t Framework::sum_failed()
{
  uint32_t count= 0;
  for (std::vector<Collection*>::iterator iter= _collection.begin();
       iter != _collection.end();
       ++iter)
  {
    count+= (*iter)->failed();
  }

  return count;
}

libtest::Runner *Framework::runner()
{
  if (_runner == NULL)
  {
    _runner= new Runner;
  }
  _runner->set_servers(_servers);

  return _runner;
}

test_return_t Framework::create()
{
  test_return_t rc= TEST_SUCCESS;
  if (_create)
  {
    _creators_ptr= _create(_servers, rc);
  }

  return rc;
}

} // namespace libtest
