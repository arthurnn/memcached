
require 'mkmf'

HERE = File.expand_path(File.dirname(__FILE__))
BUNDLE = Dir.glob("libmemcached-*.tar.gz").first
BUNDLE_PATH = BUNDLE.sub(".tar.gz", "")

$includes = ENV['INCLUDE_PATH'].to_s.split(':').map{|s| " -I#{s}"}.uniq.join

if !ENV["EXTERNAL_LIB"]
  $includes = " -I#{HERE}/include" + $includes
  $libraries = " -L#{HERE}/lib"

  puts "Building libmemcached."
  Dir.chdir(HERE) do
    puts(cmd = "tar xzf #{BUNDLE}")
    $stdout.write `#{cmd}`    
    Dir.chdir(BUNDLE_PATH) do
      puts(cmd = "./configure --prefix=#{HERE}") 
      $stdout.write `#{cmd}`      
      puts(cmd = "make 2>&1")
      $stdout.write `#{cmd}`
      puts(cmd = "make install 2>&1")
      $stdout.write `#{cmd}`
    end
    
    system("rm -rf #{BUNDLE_PATH}")
  end
end

if ENV['SWIG']
  puts "Running SWIG."
  puts(cmd = "swig #{$includes} -ruby -autorename rlibmemcached.i")  
  res = `#{cmd}`
  raise "SWIG failure" if res.match(/rlibmemcached.i:\d+: Error:/)
  $stdout.write res
end

$CFLAGS << $includes.to_s << $libraries.to_s
$LDFLAGS << $libraries.to_s

if `uname -sp` == "Darwin i386\n"
  $CFLAGS.gsub! /-arch \S+/, ''
  $CFLAGS << " -arch i386"
  $LDFLAGS.gsub! /-arch \S+/, ''
  $LDFLAGS << " -arch i386"
end

$CFLAGS.gsub! /-O\d/, ''

if ENV['DEBUG']
  puts "Setting debug flags."
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
else
  $CFLAGS << " -O3"
end

find_library(*['memcached', 'memcached_server_add_with_weight', dir_config('libmemcached').last].compact) or
  raise "Shared library 'libmemcached' not found."

[ 'libmemcached/visibility.h',
  'libmemcached/memcached.h',
  'libmemcached/memcached_constants.h',
  'libmemcached/memcached_storage.h',
  'libmemcached/memcached_result.h',
  'libmemcached/memcached_server.h'
].each do |header|
    find_header(*[header, dir_config('libmemcached').first].compact) or
      raise "Header file '#{header}' not  found."
end

create_makefile 'rlibmemcached'
