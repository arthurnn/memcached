
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
  raise "libmemcached 0.32 required; you somehow linked to #{Lib.memcached_lib_version}." unless "0.32" == Lib.memcached_lib_version
  VERSION = File.read("#{File.dirname(__FILE__)}/../CHANGELOG")[/v([\d\.]+)\./, 1]
end

require 'memcached/exceptions'
require 'memcached/behaviors'
require 'memcached/auth'
require 'memcached/marshal_codec'
require 'memcached/memcached'
require 'memcached/rails'
require 'memcached/experimental'
