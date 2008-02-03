
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'rubygems'
require 'memcached'
require 'ruby-prof'

result = RubyProf.profile do  
  load "#{HERE}/valgrind.rb"
end

printer = RubyProf::GraphPrinter.new(result)
printer.print(STDOUT, 0)
