require 'mkmf'
require 'rbconfig'

unless find_header('sasl/sasl.h')
    abort 'Please install SASL to continue. The package is called libsasl2-dev on Ubuntu and cyrus-sasl on Gentoo.'
end

HERE = File.expand_path(File.dirname(__FILE__))
BUNDLE_PATH = Dir.glob("libmemcached-*").first

SOLARIS_32 = RbConfig::CONFIG['target'] == "i386-pc-solaris2.10"
BSD = RbConfig::CONFIG['host_os'].downcase =~ /bsd/

$CFLAGS = "#{RbConfig::CONFIG['CFLAGS']} #{$CFLAGS}".gsub("$(cflags)", "").gsub("-fno-common", "").gsub("-Werror=declaration-after-statement", "")
$CFLAGS << " -std=gnu99" if SOLARIS_32
$CFLAGS << " -I/usr/local/include" if BSD
$EXTRA_CONF = " --disable-64bit" if SOLARIS_32
$LDFLAGS = "#{RbConfig::CONFIG['LDFLAGS']} #{$LDFLAGS} -L#{RbConfig::CONFIG['libdir']}".gsub("$(ldflags)", "").gsub("-fno-common", "")
$CXXFLAGS = "#{RbConfig::CONFIG['CXXFLAGS']} -std=gnu++98"
$CC = "CC=#{RbConfig::MAKEFILE_CONFIG["CC"].inspect}"

# JRuby's default configure options can't build libmemcached properly
LIBM_CFLAGS = defined?(JRUBY_VERSION) ? "-fPIC -g -O2" : $CFLAGS
LIBM_LDFLAGS = defined?(JRUBY_VERSION) ? "-fPIC -lsasl2 -lm" : $LDFLAGS

GMAKE_CMD = RbConfig::CONFIG['host_os'].downcase =~ /bsd|solaris/ ? "gmake" : "make"
TAR_CMD = SOLARIS_32 ? 'gtar' : 'tar'
PATCH_CMD = SOLARIS_32 ? 'gpatch' : 'patch'

if ENV['DEBUG']
  puts "Setting debug flags."
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
  $EXTRA_CONF = ""
end

def check_libmemcached
  return if ENV["EXTERNAL_LIB"]

  $includes = " -I#{HERE}/include"
  $libraries = " -L#{HERE}/lib"
  $CFLAGS = "#{$includes} #{$libraries} #{$CFLAGS}"
  $LDFLAGS = "-lsasl2 -lm #{$libraries} #{$LDFLAGS}"
  $LIBPATH = ["#{HERE}/lib"]
  $DEFLIBPATH = [] unless SOLARIS_32

  Dir.chdir(HERE) do
    Dir.chdir(BUNDLE_PATH) do
      ts_now=Time.now.strftime("%Y%m%d%H%M.%S")
      run("find . | xargs touch -t #{ts_now}", "Touching all files so autoconf doesn't run.")
      run("env CFLAGS='-fPIC #{LIBM_CFLAGS}' LDFLAGS='-fPIC #{LIBM_LDFLAGS}' ./configure --prefix=#{HERE} --libdir=#{HERE}/lib --without-memcached --disable-shared --disable-utils --disable-dependency-tracking #{$CC} #{$EXTRA_CONF} 2>&1", "Configuring libmemcached.")
    end

    Dir.chdir(BUNDLE_PATH) do
      #Running the make command in another script invoked by another shell command solves the "cd ." issue on FreeBSD 6+
      run("GMAKE_CMD='#{GMAKE_CMD}' CXXFLAGS='#{$CXXFLAGS} #{LIBM_CFLAGS}' SOURCE_DIR='#{BUNDLE_PATH}' HERE='#{HERE}' ruby ../extconf-make.rb", "Making libmemcached.")
    end
  end

  # Absolutely prevent the linker from picking up any other libmemcached
  Dir.chdir("#{HERE}/lib") do
    system("cp -f libmemcached.a libmemcached_gem.a")
    system("cp -f libmemcached.la libmemcached_gem.la")
  end
  $LIBS << " -lmemcached_gem -lsasl2"
end

def run(cmd, reason)
  puts reason
  puts cmd
  raise "'#{cmd}' failed" unless system(cmd)
end

check_libmemcached

$CFLAGS << " -Os"
create_makefile 'rlibmemcached'
run("mv Makefile Makefile.in", "Copy Makefile")
run("sed 's/-I.opt.local.include//' Makefile.in > Makefile", "Remove MacPorts from the include path")
