$LOAD_PATH << "#{File.dirname(__FILE__)}/../lib"

require 'socket'
require 'benchmark'

require 'rubygems'
require 'ruby-debug' if ENV['DEBUG']
require 'memcached'

require 'test/unit'
require 'test/unit/assertions'
require 'mocha/test_unit'
require 'ostruct'

UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached') unless defined? UNIX_SOCKET_NAME

class GenericClass
end
