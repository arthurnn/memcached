
require "#{File.dirname(__FILE__)}/../setup"

$LOAD_PATH << "#{File.dirname(__FILE__)}/../../lib/"
require 'rubygems'
require 'memcached'
require 'ruby-prof'

result = RubyProf.profile do  
  load "#{HERE}/profile/valgrind.rb"
end

printer = RubyProf::GraphPrinter.new(result)
printer.print(STDOUT, 0)
