
require 'mkmf'

HERE = File.expand_path(File.dirname(__FILE__))
BUNDLE = Dir.glob("libmemcached-*.tar.gz").first
BUNDLE_PATH = BUNDLE.sub(".tar.gz", "")

DARWIN = `uname -sp` == "Darwin i386\n"

if !ENV["EXTERNAL_LIB"]
  $includes = " -I#{HERE}/include"
  $libraries = " -L#{HERE}/lib"

  $CFLAGS = "#{$includes} #{$libraries} #{$CFLAGS}"
  $LDFLAGS = "#{$libraries} #{$LDFLAGS}"
  $CPPFLAGS = $ARCH_FLAG = $DLDFLAGS = ""

  $LIBPATH = ["#{HERE}/lib"]
  $DEFLIBPATH = []

  Dir.chdir(HERE) do
    if File.exist?("lib")
      puts "Libmemcached already built; run 'rake clean' first if you need to rebuild."
    else
      puts "Building libmemcached."
      puts(cmd = "tar xzf #{BUNDLE} 2>&1")
      raise "'#{cmd}' failed" unless system(cmd)

      Dir.chdir(BUNDLE_PATH) do
        puts(cmd = "env CFLAGS=-fPIC LDFLAGS=-fPIC ./configure --prefix=#{HERE} --without-memcached --disable-shared --disable-utils 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)
        puts(cmd = "make || true 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)
        puts(cmd = "make install || true 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)
      end
      system("rm -rf #{BUNDLE_PATH}")
    end
  end
  
  # Absolutely prevent the linker from picking up any other libmemcached
  Dir.chdir("#{HERE}/lib") do
    system("cp -f libmemcached.a libmemcached_gem.a") 
    system("cp -f libmemcached.la libmemcached_gem.la") 
  end
  $LIBS << " -lmemcached_gem"
end

if DARWIN
  $CFLAGS.gsub! /-arch \S+/, ''
  $CFLAGS << " -arch i386"
  $LDFLAGS.gsub! /-arch \S+/, ''
  $LDFLAGS << " -arch i386"
end

if ENV['SWIG']
  puts "Running SWIG."
  puts(cmd = "swig #{$includes} -ruby -autorename rlibmemcached.i")
  raise "'#{cmd}' failed" unless system(cmd)
end

$CFLAGS.gsub! /-O\d/, ''

if ENV['DEBUG']
  puts "Setting debug flags."
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
else
  $CFLAGS << " -O3"
end

create_makefile 'rlibmemcached'
