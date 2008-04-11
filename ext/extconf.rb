
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
raise "shared library 'libmemcached' not found" unless 
  find_library('memcached', 'memcached_server_add', *ENV['LD_LIBRARY_PATH'].to_s.split(":"))

[ 
  'libmemcached/memcached.h',
  'libmemcached/memcached_constants.h', 
  'libmemcached/memcached_storage.h',
  'libmemcached/memcached_result.h',
  'libmemcached/memcached_server.h'
].each do |header|
  raise "header file '#{header}' not  found" unless 
    find_header(header, *ENV['INCLUDE_PATH'].to_s.split(":"))
end

create_makefile 'rlibmemcached'
