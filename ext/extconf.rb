
require 'mkmf'

if ENV['SWIG']
  puts "running SWIG"
  $stdout.write `swig -I/opt/local/include -ruby -autorename rlibmemcached.i`
end

$CFLAGS.gsub! /-O\d/, ''

if ENV['DEBUG']
  puts "setting debug flags"
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG" 
else
  $CFLAGS << " -O3"
end

dir_config 'rlibmemcached'

# XXX There's probably a better way to do this
unless find_library 'memcached', 'memcached_server_add', *ENV['LD_LIBRARY_PATH'].to_s.split(":")
  raise "shared library 'libmemcached' not found"
end

['libmemcached/memcached.h', 'libmemcached/memcached_constants.h'].each do |include|
  raise "header file '#{include}' not  found" unless find_header include, *ENV['INCLUDE_PATH'].to_s.split(":")
end

create_makefile 'rlibmemcached'
