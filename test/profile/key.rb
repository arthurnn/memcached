
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

#result = RubyProf.profile do  
Benchmark.bm do |x|
  x.report("rlibmemcached") do
    KEYS.each do |key|
      10000.times do
        Rlibmemcached.ns(@namespace, key)
      end
    end
  end
  #  x.report("instance_eval") do
  #    @cache.instance_eval do 
  #      KEYS.each do |key|
  #        10000.times do
  #          ns(key)
  #        end
  #      end  
  #    end
  #  end
end

#printer = RubyProf::GraphPrinter.new(result)
#printer.print(STDOUT, 0)
