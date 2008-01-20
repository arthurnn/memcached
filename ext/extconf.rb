
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
find_library 'memcached', 'memcached_server_add'
create_makefile 'libmemcached'
