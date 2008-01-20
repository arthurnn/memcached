
require 'libmemcached'
require 'memcached/integer'
require 'memcached/exceptions'
require 'memcached/behaviors'
require 'memcached/memcached'

=begin rdoc
The generated SWIG module for accessing libmemcached's C API.

Includes the full set of libmemcached static methods (as defined in <tt>$INCLUDE_PATH/libmemcached/memcached.h</tt>), and classes for the available structs:

* Libmemcached::MemcachedResultSt
* Libmemcached::MemcachedServerSt
* Libmemcached::MemcachedSt
* Libmemcached::MemcachedStatSt
* Libmemcached::MemcachedStringSt

A number of SWIG typemaps and C helper methods are also defined in <tt>ext/libmemcached.i</tt>.

=end
module Libmemcached
end