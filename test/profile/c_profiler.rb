require "#{File.dirname(__FILE__)}/../setup"

$LOAD_PATH << "#{File.dirname(__FILE__)}/../../lib/"
require 'memcached'

profile = "/tmp/memcached_#{Memcached::VERSION}_c"

system("env CPUPROFILE_FREQUENCY=500 CPUPROFILE=#{profile}.out DYLD_INSERT_LIBRARIES=/opt/local/lib/libprofiler.dylib ruby -r#{File.dirname(__FILE__)}/exercise -e \"Worker.new('mixed', 200000).work\"")

ruby = `which ruby`.chomp

system("pprof --nodefraction=0.0000001 --text #{ruby} #{profile}.out")
system("pprof --nodefraction=0.0000001 --pdf #{ruby} #{profile}.out > #{profile}.pdf")
system("open #{profile}.pdf")
