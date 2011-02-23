
require "#{File.dirname(__FILE__)}/../setup"

$LOAD_PATH << "#{File.dirname(__FILE__)}/../../lib/"
require 'rubygems'
require 'memcached'
require 'perftools'
require "#{HERE}/profile/exercise"

profile = "/tmp/memcached_#{Memcached::VERSION}"
worker = Worker.new('mixed', 500000, true)

PerfTools::CpuProfiler.start("#{profile}.out") do
  worker.work
end

system("pprof.rb --focus=Memcached --text #{profile}.out")
system("pprof.rb --focus=Memcached --pdf #{profile}.out > #{profile}.pdf")
system("open #{profile}.pdf")
