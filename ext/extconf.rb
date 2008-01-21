
require 'mkmf'

if ENV['SWIG']
  puts "running SWIG"
  $stdout.write `swig -I/opt/local/include -ruby -autorename libmemcached.i`
end

if ENV['DEBUG']
  puts "setting debug flags"
  $CFLAGS << " -ggdb -DHAVE_DEBUG" 
end

dir_config 'libmemcached'

# XXX There's probably a better way to do this
unless find_library 'memcached', 'memcached_server_add', *ENV['LD_LIBRARY_PATH'].to_s.split(":")
  raise "shared library 'libmemcached' not found"
end
unless find_header 'libmemcached/memcached.h', *ENV['INCLUDE_PATH'].to_s.split(":")
  raise "header file 'libmemcached/memcached.h' not  found"
end

create_makefile 'libmemcached'
