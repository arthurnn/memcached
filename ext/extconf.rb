require 'mkmf'
require 'rbconfig'

HERE = File.expand_path(File.dirname(__FILE__))
BUNDLE_PATH = Dir.glob("libmemcached-*").first

SOLARIS_32 = RbConfig::CONFIG['target'] == "i386-pc-solaris2.10"
BSD = Config::CONFIG['host_os'].downcase =~ /bsd/

$CFLAGS = "#{RbConfig::CONFIG['CFLAGS']} #{$CFLAGS}".gsub("$(cflags)", "").gsub("-fno-common", "")
$CFLAGS << " -std=gnu99" if SOLARIS_32
$CFLAGS << " -I/usr/local/include" if BSD
$EXTRA_CONF = " --disable-64bit" if SOLARIS_32
$LDFLAGS = "#{RbConfig::CONFIG['LDFLAGS']} #{$LDFLAGS} -L#{RbConfig::CONFIG['libdir']}".gsub("$(ldflags)", "").gsub("-fno-common", "")
$CXXFLAGS = " -std=gnu++98"
$CPPFLAGS = $ARCH_FLAG = $DLDFLAGS = ""

# JRuby's default configure options can't build libmemcached properly
LIBM_CFLAGS = defined?(JRUBY_VERSION) ? "-fPIC -g -O2" : $CFLAGS
LIBM_LDFLAGS = defined?(JRUBY_VERSION) ? "-fPIC -lsasl2 -lm" : $LDFLAGS

GMAKE_CMD = Config::CONFIG['host_os'].downcase =~ /bsd|solaris/ ? "gmake" : "make"
TAR_CMD = SOLARIS_32 ? 'gtar' : 'tar'
PATCH_CMD = SOLARIS_32 ? 'gpatch' : 'patch'

if ENV['DEBUG']
  puts "Setting debug flags."
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
  $EXTRA_CONF = ""
end

def run(cmd, reason)
  puts reason
  puts cmd
  raise "'#{cmd}' failed" unless system(cmd)
end

def check_libmemcached
  return if ENV["EXTERNAL_LIB"]

  $includes = " -I#{HERE}/include"
  $defines = " -DLIBMEMCACHED_WITH_SASL_SUPPORT"
  $libraries = " -L#{HERE}/lib"
  $CFLAGS = "#{$includes} #{$libraries} #{$CFLAGS}"
  $LDFLAGS = "-lsasl2 -lm #{$libraries} #{$LDFLAGS}"
  $LIBPATH = ["#{HERE}/lib"]
  $DEFLIBPATH = [] unless SOLARIS_32

  Dir.chdir(HERE) do
    run("touch -r #{BUNDLE_PATH}/m4/visibility.m4 #{BUNDLE_PATH}/configure.ac #{BUNDLE_PATH}/m4/pandora_have_sasl.m4", "Touching aclocal.m4 in libmemcached.")

    Dir.chdir(BUNDLE_PATH) do
      run("env CFLAGS='-fPIC #{LIBM_CFLAGS}' LDFLAGS='-fPIC #{LIBM_LDFLAGS}' ./configure --prefix=#{HERE} --without-memcached --disable-shared --disable-utils --disable-dependency-tracking #{$EXTRA_CONF} 2>&1", "Configuring libmemcached.")
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

check_libmemcached

if ENV['SWIG']
  puts "WARNING: Swig 2.0.2 not found. Other versions may not work." if (`swig -version`!~ /2.0.2/)
  run("swig #{$defines} #{$includes} -ruby -autorename rlibmemcached.i", "Running SWIG.")
  run("sed -i '' 's/STR2CSTR/StringValuePtr/' rlibmemcached_wrap.c", "Patching SWIG output for Ruby 1.9.")
  run("sed -i '' 's/\"swig_runtime_data\"/\"SwigRuntimeData\"/' rlibmemcached_wrap.c", "Patching SWIG output for Ruby 1.9.")
  run("sed -i '' 's/#ifndef RUBY_INIT_STACK/#ifdef __NEVER__/g' rlibmemcached_wrap.c", "Patching SWIG output for JRuby.")
end

$CFLAGS << " -Os"
create_makefile 'rlibmemcached'
