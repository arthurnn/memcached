
if `which swig` !~ /no swig/
  puts "running SWIG"
  $stdout.write `swig -I/opt/local/include -ruby -autorename libmemcached.i`
end

require 'mkmf'
$CFLAGS << " -ggdb -DHAVE_DEBUG"
dir_config 'libmemcached'
# find_header 'libmemcached/memcached.h'
find_library 'memcached', 'memcached_server_add'
create_makefile 'libmemcached'
