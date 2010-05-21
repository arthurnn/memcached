require 'mkmf'
require 'rbconfig'

HERE = File.expand_path(File.dirname(__FILE__))
BUNDLE = Dir.glob("libmemcached-*.tar.gz").first
BUNDLE_PATH = BUNDLE.sub(".tar.gz", "")

$CFLAGS = "#{RbConfig::CONFIG['CFLAGS']} #{$CFLAGS}".gsub("$(cflags)", "").gsub("-fno-common", "")
$LDFLAGS = "#{RbConfig::CONFIG['LDFLAGS']} #{$LDFLAGS}".gsub("$(ldflags)", "").gsub("-fno-common", "")
$CXXFLAGS = " -std=gnu++98 #{$CFLAGS}"
$CPPFLAGS = $ARCH_FLAG = $DLDFLAGS = ""

if ENV['DEBUG']
  puts "Setting debug flags."
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
  $EXTRA_CONF = ""
end

def check_libmemcached
  return if ENV["EXTERNAL_LIB"]

  $includes = " -I#{HERE}/include"
  $defines = " -DLIBMEMCACHED_WITH_SASL_SUPPORT"
  $libraries = " -L#{HERE}/lib"
  $CFLAGS = "#{$includes} #{$libraries} #{$CFLAGS}"
  $LDFLAGS = "#{$libraries} #{$LDFLAGS}"
  $LIBPATH = ["#{HERE}/lib"]
  $DEFLIBPATH = []

  Dir.chdir(HERE) do
    if File.exist?("lib")
      puts "Libmemcached already built; run 'rake clean' first if you need to rebuild."
    else

      # have_sasl check may fail on OSX, skip it
      # unless RUBY_PLATFORM =~ /darwin/ or have_library('sasl2')
      #   raise "SASL2 not found. You need the libsasl2-dev library, which should be provided through your system's package manager."
      # end

      puts "Building libmemcached."
      puts(cmd = "tar xzf #{BUNDLE} 2>&1")
      raise "'#{cmd}' failed" unless system(cmd)

      puts "Patching libmemcached source."
      puts(cmd = "patch -p1 -Z < libmemcached.patch")
      raise "'#{cmd}' failed" unless system(cmd)

      puts "Patching libmemcached with SASL support."
      puts(cmd = "patch -p1 -Z < sasl.patch")
      raise "'#{cmd}' failed" unless system(cmd)

      puts "Touching aclocal.m4  in libmemcached."
      puts(cmd = "touch -r #{BUNDLE_PATH}/m4/visibility.m4 #{BUNDLE_PATH}/configure.ac #{BUNDLE_PATH}/m4/pandora_have_sasl.m4")
      raise "'#{cmd}' failed" unless system(cmd)

      Dir.chdir(BUNDLE_PATH) do
        puts(cmd = "env CFLAGS='-fPIC #{$CFLAGS}' LDFLAGS='-fPIC #{$LDFLAGS}' ./configure --prefix=#{HERE} --without-memcached --disable-shared --disable-utils --disable-dependency-tracking #{$EXTRA_CONF} 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)

        puts(cmd = "make CXXFLAGS='#{$CXXFLAGS}' || true 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)

        puts(cmd = "make install || true 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)
      end

      system("rm -rf #{BUNDLE_PATH}") unless ENV['DEBUG'] or ENV['DEV']
    end
  end

  # Absolutely prevent the linker from picking up any other libmemcached
  Dir.chdir("#{HERE}/lib") do
    system("cp -f libmemcached.a libmemcached_gem.a")
    system("cp -f libmemcached.la libmemcached_gem.la")
  end
  $LIBS << " -lmemcached_gem -lsasl2"
end

if ENV['SWIG']
  puts "Running SWIG."
  puts(cmd = "swig #{$defines} #{$includes} -ruby -autorename rlibmemcached.i")
  raise "'#{cmd}' failed" unless system(cmd)
  puts(cmd = "sed -i 's/STR2CSTR/StringValuePtr/' rlibmemcached_wrap.c")
  raise "'#{cmd}' failed" unless system(cmd)
end

check_libmemcached

create_makefile 'rlibmemcached'
