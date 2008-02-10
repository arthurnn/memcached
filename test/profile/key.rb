
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'rubygems'
require 'memcached'
require 'ruby-prof'
require 'benchmark'

KEYS = ["ch\303\242teau"*10, "long"*100, "spaces space spaces", "null \000"]

@namespace = "benchmark_namespace"
@cache = Memcached.new(
  ['127.0.0.1:43042', '127.0.0.1:43043'], 
   :namespace => @namespace
 )

def dispatch(n, k)
  Rlibmemcached.ns(@namespace, key)
end

#result = RubyProf.profile do  
Benchmark.bm(15) do |x|
  x.report("direct") do
    KEYS.each do |key|
      10000.times do
        Rlibmemcached.ns(@namespace, key)
      end
    end
  end
  x.report("dispatch") do
    KEYS.each do |key|
      10000.times do
        Rlibmemcached.ns(@namespace, key)
      end
    end
  end
end

#printer = RubyProf::GraphPrinter.new(result)
#printer.print(STDOUT, 0)
