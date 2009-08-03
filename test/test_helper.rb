
$LOAD_PATH << "#{File.dirname(__FILE__)}/../lib"

require 'rubygems'
require 'mocha'

if ENV['DEBUG']
  require 'ruby-debug' 
end
  
require 'memcached'
require 'test/unit'
require 'ostruct'

UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached') 

class GenericClass
end
