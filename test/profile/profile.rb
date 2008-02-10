
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'rubygems'
require 'memcached'
require 'ruby-prof'

@cache = Memcached.new(
  '127.0.0.1:43042', '127.0.0.1:43043'], 
   :namespace => "benchmark_namespace"
 )

result = RubyProf.profile do  
  load "#{HERE}/valgrind.rb"
end

printer = RubyProf::GraphPrinter.new(result)
printer.print(STDOUT, 0)
