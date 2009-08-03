
require 'mkmf'

HERE = File.expand_path(File.dirname(__FILE__))

INCLUDES = ENV['INCLUDE_PATH'].to_s.split(':').map{|s| " -I#{s}"}.uniq.join

unless ENV['EXTERNAL_LIB']
  puts "building memcache"
  Dir.chdir(HERE) do
    cmd = "cd libmemcached-src && ./configure --prefix=#{HERE}/unused --libdir=#{HERE}/.. --includedir=#{HERE}/..
  make && make install"
    puts cmd
    res = `#{cmd}`
    $stdout.write res
  end
  INCLUDES += " -I./libmemcache-include -L./libmemcache-lib"
end

if ENV['SWIG']
  puts "running SWIG"
  cmd = "swig #{INCLUDES} -ruby -autorename rlibmemcached.i"
  puts cmd
  res = `#{cmd}`
  raise "SWIG failure" if res.match(/rlibmemcached.i:\d+: Error:/)
  $stdout.write res
end

$CFLAGS << INCLUDES

if `uname -sp` == "Darwin i386\n"
  $CFLAGS.gsub! /-arch \S+/, ''
  $CFLAGS << " -arch i386"
  $LDFLAGS.gsub! /-arch \S+/, ''
  $LDFLAGS << " -arch i386"
end

$CFLAGS.gsub! /-O\d/, ''

if ENV['DEBUG']
  puts "setting debug flags"
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
else
  $CFLAGS << " -O3"
end

find_library(*['memcached', 'memcached_server_add_with_weight', dir_config('libmemcached').last].compact) or
  raise "shared library 'libmemcached' not found"

[
 'libmemcached/visibility.h',
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
