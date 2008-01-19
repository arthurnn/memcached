
$LOAD_PATH << "#{File.dirname(__FILE__)}/../lib"
require 'memcached'

require 'rubygems'
require 'test/unit'
require 'ostruct'

class GenericClass
end

class Memcached
  class StubError < Error
  end
end