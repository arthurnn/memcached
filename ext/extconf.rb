
require 'mkmf'

if ENV['SWIG']
  puts "running SWIG"
  $stdout.write `swig -I/opt/local/include -ruby -autorename rlibmemcached.i`
end

if ENV['DEBUG']
  puts "setting debug flags"
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG" 
else
  $CFLAGS << " -O2"
end

dir_config 'rlibmemcached'

# XXX There's probably a better way to do this
unless find_library 'memcached', 'memcached_server_add', *ENV['LD_LIBRARY_PATH'].to_s.split(":")
  raise "shared library 'libmemcached' not found"
end
unless find_header 'libmemcached/memcached.h', *ENV['INCLUDE_PATH'].to_s.split(":")
  raise "header file 'libmemcached/memcached.h' not  found"
end

create_makefile 'rlibmemcached'
