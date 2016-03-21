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

#include <libtest/gearmand.h>

using namespace libtest;

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

using namespace libtest;

class Gearmand : public libtest::Server
{
private:
public:
  Gearmand(const std::string& host_arg, in_port_t port_arg, bool libtool_, const char* binary);

  bool ping()
  {
    reset_error();

    if (out_of_ban_killed())
    {
      return false;
    }

    SimpleClient client(_hostname, _port);

    std::string response;
    bool ret= client.send_message("version", response);

    if (client.is_error())
    {
      error(client.error());
    }

    return ret;
  }

  const char *name()
  {
    return "gearmand";
  };

  void log_file_option(Application& app, const std::string& arg)
  {
    if (arg.empty() == false)
    {
      std::string buffer("--log-file=");
      buffer+= arg;
      app.add_option("--verbose=DEBUG");
      app.add_option(buffer);
    }
  }

  bool has_log_file_option() const
  {
    return true;
  }

  bool is_libtool()
  {
    return true;
  }

  bool has_syslog() const
  {
    return false; //  --syslog.errmsg-enable
  }

  bool has_port_option() const
  {
    return true;
  }

  bool build();
};

Gearmand::Gearmand(const std::string& host_arg, in_port_t port_arg, bool libtool_, const char* binary_arg) :
  libtest::Server(host_arg, port_arg, binary_arg, libtool_)
{
  set_pid_file();
}

bool Gearmand::build()
{
  if (getuid() == 0 or geteuid() == 0)
  {
    add_option("-u", "root");
  }

  add_option("--listen=localhost");

  return true;
}

namespace libtest {

libtest::Server *build_gearmand(const char *hostname, in_port_t try_port, const char* binary)
{
  if (binary == NULL)
  {
#if defined(HAVE_GEARMAND_BINARY)
# if defined(GEARMAND_BINARY)
    if (HAVE_GEARMAND_BINARY)
    {
      binary= GEARMAND_BINARY;
    }
# endif
#endif
  }

  if (binary == NULL)
  {
    return NULL;
  }

  bool is_libtool_script= true;

  if (binary[0] == '/')
  {
    is_libtool_script= false;
  }

  return new Gearmand(hostname, try_port, is_libtool_script, binary);
}

}
