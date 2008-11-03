
require 'mkmf'

if ENV['SWIG']
  puts "running SWIG"
  $stdout.write `swig -I/opt/local/include -ruby -autorename rlibmemcached.i`
end

if `uname -sp` == "Darwin i386\n"
  ENV['ARCHFLAGS'] = "-arch i386"
end

$CFLAGS.gsub! /-O\d/, ''

if ENV['DEBUG']
  puts "setting debug flags"
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG" 
else
  $CFLAGS << " -O3"
end

p dir_config('libmemcached')

find_library(*['memcached', 'memcached_server_add_with_weight', dir_config('libmemcached').last].compact) or
  raise "shared library 'libmemcached' not found"

[ 
  'libmemcached/memcached.h',
  'libmemcached/memcached_constants.h', 
  'libmemcached/memcached_storage.h',
  'libmemcached/memcached_result.h',
  'libmemcached/memcached_server.h'
].each do |header|  
    find_header(*[header, dir_config('libmemcached').first].compact) or
      raise "header file '#{header}' not  found"
end

create_makefile 'rlibmemcached'
