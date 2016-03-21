require 'mkmf'
require 'rbconfig'

unless find_header('sasl/sasl.h')
  abort 'Please install SASL to continue. The package is called libsasl2-dev on Ubuntu and cyrus-sasl on Gentoo.'
end

HERE = File.expand_path(File.dirname(__FILE__))
LIBMEMCACHED_DIR = Dir.glob(File.join(HERE, '..', '..', 'vendor',"libmemcached-*")).first

SOLARIS_32 = RbConfig::CONFIG['target'] == "i386-pc-solaris2.10"
BSD = RbConfig::CONFIG['host_os'].downcase =~ /bsd/

$CFLAGS = "#{RbConfig::CONFIG['CFLAGS']} #{$CFLAGS}".gsub("$(cflags)", "").gsub("-fno-common", "").gsub("-Wdeclaration-after-statement", "")
$CFLAGS << " -Os"
$CFLAGS << " -std=gnu99" if SOLARIS_32
$CFLAGS << " -I/usr/local/include" if BSD
$EXTRA_CONF = " --disable-64bit" if SOLARIS_32
$LDFLAGS = "#{RbConfig::CONFIG['LDFLAGS']} #{$LDFLAGS} -L#{RbConfig::CONFIG['libdir']}".gsub("$(ldflags)", "").gsub("-fno-common", "")
$CXXFLAGS = "#{RbConfig::CONFIG['CXXFLAGS']} -fPIC"
$CC = "CC=#{RbConfig::MAKEFILE_CONFIG["CC"].inspect}"

# JRuby's default configure options can't build libmemcached properly
LIBM_CFLAGS = defined?(JRUBY_VERSION) ? "-fPIC -g -O2" : $CFLAGS
LIBM_LDFLAGS = defined?(JRUBY_VERSION) ? "-fPIC -lsasl2 -lm" : $LDFLAGS

GMAKE_CMD = RbConfig::CONFIG['host_os'].downcase =~ /bsd|solaris/ ? "gmake" : "make"

if ENV['DEBUG']
  puts "Setting debug flags."
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
  $EXTRA_CONF = ""
end

def compile_libmemcached
  return if ENV["EXTERNAL_LIB"]

  Dir.chdir(LIBMEMCACHED_DIR) do
    Dir.mkdir("build") if !Dir.exists?("build")
    build_folder = File.join(LIBMEMCACHED_DIR, "build")

    ts_now=Time.now.strftime("%Y%m%d%H%M.%S")
    run("find . | xargs touch -t #{ts_now}", "Touching all files so autoconf doesn't run.")
    run("env CFLAGS='-fPIC #{LIBM_CFLAGS}' LDFLAGS='-fPIC #{LIBM_LDFLAGS}' ./configure --prefix=#{build_folder} --libdir=#{build_folder}/lib --with-pic --without-memcached --disable-shared --disable-util\
s --disable-dependency-tracking #{$CC} #{$EXTRA_CONF} 2>&1", "Configuring libmemcached.")
    run("#{GMAKE_CMD} clean 2>&1")
    run("#{GMAKE_CMD} CXXFLAGS='#{$CXXFLAGS}' 2>&1")
    run("#{GMAKE_CMD} install 2>&1")

    pcfile = File.join(LIBMEMCACHED_DIR, "build", "lib", "pkgconfig", "libmemcached.pc")
    $LDFLAGS << " -lsasl2 " + `pkg-config --libs --static #{pcfile}`.strip
  end
  $DEFLIBPATH.unshift("#{LIBMEMCACHED_DIR}/build")
  dir_config('memcached', "#{LIBMEMCACHED_DIR}/build/include", "#{LIBMEMCACHED_DIR}/build/lib")
end

def run(cmd, reason = nil)
  puts reason if reason
  puts cmd
  raise "'#{cmd}' failed" unless system(cmd)
end

compile_libmemcached

$libs = append_library($libs, "stdc++")

unless have_library 'memcached' and have_header 'libmemcached/memcached.h'
  abort "ERROR: Failed to build libmemcached"
end

create_makefile 'memcached'
