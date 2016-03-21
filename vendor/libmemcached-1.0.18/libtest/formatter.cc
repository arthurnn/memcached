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

#include <algorithm>
#include <fstream>
#include <iostream>
  
namespace libtest {

std::string& escape4XML(std::string const& arg, std::string& escaped_string)
{
  escaped_string.clear();

  escaped_string+= '"';
  for (std::string::const_iterator x= arg.begin(), end= arg.end(); x != end; ++x)
  {
    unsigned char c= *x;
    if (c == '&')
    {
      escaped_string+= "&amp;";
    }
    else if (c == '>')
    {
      escaped_string+= "&gt;";
    }
    else if (c == '<')
    {
      escaped_string+= "&lt;";
    }
    else if (c == '\'')
    {
      escaped_string+= "&apos;";  break;
    }
    else if (c == '"')
    {
      escaped_string+= "&quot;";
    }
    else if (c == ' ')
    {
      escaped_string+= ' ';
    }
    else if (isalnum(c))
    {
      escaped_string+= c;
    }
    else 
    {
      char const* const hexdig= "0123456789ABCDEF";
      escaped_string+= "&#x";
      escaped_string+= hexdig[c >> 4];
      escaped_string+= hexdig[c & 0xF];
      escaped_string+= ';';
    }
  }
  escaped_string+= '"';

  return escaped_string;
}

class TestCase {
public:
  TestCase(const std::string& arg):
    _name(arg),
    _result(TEST_FAILURE)
  {
  }

  const std::string& name() const
  {
    return _name;
  }

  test_return_t result() const
  {
    return _result;
  }

  void result(test_return_t arg)
  {
    _result= arg;
  }

  void result(test_return_t arg, const libtest::Timer& timer_)
  {
    _result= arg;
    _timer= timer_;
  }

  const libtest::Timer& timer() const
  {
    return _timer;
  }

  void timer(libtest::Timer& arg)
  {
    _timer= arg;
  }

private:
  std::string _name;
  test_return_t _result;
  libtest::Timer _timer;
};

Formatter::Formatter(const std::string& frame_name, const std::string& arg)
{
  _suite_name= frame_name;
  _suite_name+= ".";
  _suite_name+= arg;
}

Formatter::~Formatter()
{
  std::for_each(_testcases.begin(), _testcases.end(), DeleteFromVector());
  _testcases.clear();
}

TestCase* Formatter::current()
{
  return _testcases.back();
}

void Formatter::skipped()
{
  current()->result(TEST_SKIPPED);
  Out << name() << "." 
      << current()->name()
      <<  "\t\t\t\t\t" 
      << "[ " << test_strerror(current()->result()) << " ]";

  reset();
}

void Formatter::failed()
{
  assert(current());
  current()->result(TEST_FAILURE);

  Out << name()
    << "." << current()->name() <<  "\t\t\t\t\t" 
    << "[ " << test_strerror(current()->result()) << " ]";

  reset();
}

void Formatter::success(const libtest::Timer& timer_)
{
  assert(current());
  current()->result(TEST_SUCCESS, timer_);
  std::string escaped_string;

  Out << name() << "."
    << current()->name()
    <<  "\t\t\t\t\t" 
    << current()->timer() 
    << " [ " << test_strerror(current()->result()) << " ]";

  reset();
}

void Formatter::xml(libtest::Framework& framework_, std::ofstream& output)
{
  std::string escaped_string;

  output << "<testsuites name=" 
    << escape4XML(framework_.name(), escaped_string) << ">" << std::endl;

  for (Suites::iterator framework_iter= framework_.suites().begin();
       framework_iter != framework_.suites().end();
       ++framework_iter)
  {
    output << "\t<testsuite name=" 
      << escape4XML((*framework_iter)->name(), escaped_string)
#if 0
      << "  classname=\"\" package=\"\"" 
#endif
      << ">" << std::endl;

    for (TestCases::iterator case_iter= (*framework_iter)->formatter()->testcases().begin();
         case_iter != (*framework_iter)->formatter()->testcases().end();
         ++case_iter)
    {
      output << "\t\t<testcase name=" 
        << escape4XML((*case_iter)->name(), escaped_string)
        << " time=\"" 
        << (*case_iter)->timer().elapsed_milliseconds() 
        << "\""; 

      switch ((*case_iter)->result())
      {
        case TEST_SKIPPED:
        output << ">" << std::endl;
        output << "\t\t <skipped/>" << std::endl;
        output << "\t\t</testcase>" << std::endl;
        break;

        case TEST_FAILURE:
        output << ">" << std::endl;
        output << "\t\t <failure message=\"\" type=\"\"/>"<< std::endl;
        output << "\t\t</testcase>" << std::endl;
        break;

        case TEST_SUCCESS:
        output << "/>" << std::endl;
        break;
      }
    }
    output << "\t</testsuite>" << std::endl;
  }
  output << "</testsuites>" << std::endl;
}

void Formatter::push_testcase(const std::string& arg)
{
  assert(_suite_name.empty() == false);
  TestCase* _current_testcase= new TestCase(arg);
  _testcases.push_back(_current_testcase);
}

void Formatter::reset()
{
}
} // namespace libtest
