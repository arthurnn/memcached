
require "#{File.dirname(__FILE__)}/../setup"

$LOAD_PATH << "#{File.dirname(__FILE__)}/../../lib/"

ENV['CPUPROFILE_FREQUENCY'] = '500'
require 'memcached'
require 'rubygems'
require 'perftools'
require "#{HERE}/profile/exercise"

profile = "/tmp/memcached_#{Memcached::VERSION}_rb"
worker = Worker.new('mixed', 200000)

PerfTools::CpuProfiler.start("#{profile}.out") do
  worker.work
end

system("pprof.rb --nodefraction=0.0000001 --text #{profile}.out")
system("pprof.rb --nodefraction=0.0000001 --edgefraction=0.0000001 --pdf #{profile}.out > #{profile}.pdf")
system("open #{profile}.pdf")
