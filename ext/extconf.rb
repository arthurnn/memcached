
if `which swig` !~ /no swig/
  puts "running SWIG"
  $stdout.write `swig -I/opt/local/include -ruby memcached.i`
end

require 'mkmf'
dir_config 'memcached'
# find_header 'libmemcached/memcached.h'
find_library 'memcached', 'memcached_server_add'
create_makefile 'memcached'
