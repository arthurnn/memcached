
require 'libmemcached'
require 'memcached/integer'
require 'memcached/exceptions'
require 'memcached/behaviors'
require 'memcached/memcached'

if defined? ::Rails
  require 'memcached/rails'
end

=begin rdoc
The generated SWIG module for accessing libmemcached's C API.

Includes the full set of libmemcached static methods (as defined in <tt>$INCLUDE_PATH/libmemcached/memcached.h</tt>), and classes for the available structs:

* <b>Libmemcached::MemcachedResultSt</b>
* <b>Libmemcached::MemcachedServerSt</b>
* <b>Libmemcached::MemcachedSt</b>
* <b>Libmemcached::MemcachedStatSt</b>
* <b>Libmemcached::MemcachedStringSt</b>

A number of SWIG typemaps and C helper methods are also defined in <tt>ext/libmemcached.i</tt>.

=end
module Libmemcached
end