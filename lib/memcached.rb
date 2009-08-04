
=begin rdoc
The generated SWIG module for accessing libmemcached's C API.

Includes the full set of libmemcached static methods (as defined in <tt>$INCLUDE_PATH/libmemcached/memcached.h</tt>), and classes for the available structs:

* <b>Rlibmemcached::MemcachedResultSt</b>
* <b>Rlibmemcached::MemcachedServerSt</b>
* <b>Rlibmemcached::MemcachedSt</b>
* <b>Rlibmemcached::MemcachedStatSt</b>
* <b>Rlibmemcached::MemcachedStringSt</b>

A number of SWIG typemaps and C helper methods are also defined in <tt>ext/libmemcached.i</tt>.

=end
module Rlibmemcached
end

require 'rlibmemcached'

class Memcached
  Lib = Rlibmemcached
  REQUIRED_VERSION = File.read("#{File.dirname(__FILE__)}/../COMPATIBILITY")[/:: ([\d\.]+)/, 1]
  RECEIVED_VERSION = Lib.memcached_lib_version
  raise "libmemcached #{REQUIRED_VERSION} required; your gem is linked to #{RECEIVED_VERSION}." unless REQUIRED_VERSION == RECEIVED_VERSION
end

require 'memcached/integer'
require 'memcached/exceptions'
require 'memcached/behaviors'
require 'memcached/memcached'
require 'memcached/rails'
