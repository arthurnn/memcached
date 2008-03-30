
$LOAD_PATH << "#{File.dirname(__FILE__)}/../lib"

if ENV['DEBUG']
  require 'rubygems'
  require 'ruby-debug' 
end
  
require 'memcached'
require 'test/unit'
require 'ostruct'

class GenericClass
end
