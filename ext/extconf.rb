
if `which swig` !~ /no swig/
  puts "running SWIG"
  $stdout.write `swig -ruby memcached.i`
end

require 'mkmf'
dir_config 'memcached'
create_makefile 'memcached'
