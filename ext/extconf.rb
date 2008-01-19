
if `which swig` !~ /no swig/
  puts "running SWIG"
  $stdout.write `swig -I/opt/local/include -ruby -autorename libmemcached.i`
end

require 'mkmf'
# $CFLAGS << " -ggdb -DHAVE_DEBUG"
# find_header 'libmemcached/memcached.h'
dir_config 'libmemcached'
find_library 'memcached', 'memcached_server_add'
create_makefile 'libmemcached'
