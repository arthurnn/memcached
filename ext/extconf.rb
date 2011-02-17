require 'mkmf'
require 'rbconfig'

HERE = File.expand_path(File.dirname(__FILE__))
BUNDLE = Dir.glob("libmemcached-*.tar.gz").first
BUNDLE_PATH = BUNDLE.sub(".tar.gz", "")

SOLARIS_32 = RbConfig::CONFIG['target'] == "i386-pc-solaris2.10"

$CFLAGS = "#{RbConfig::CONFIG['CFLAGS']} #{$CFLAGS}".gsub("$(cflags)", "").gsub("-fno-common", "")
$CFLAGS << " -std=gnu99" if SOLARIS_32
$EXTRA_CONF = " --disable-64bit" if SOLARIS_32
$LDFLAGS = "#{RbConfig::CONFIG['LDFLAGS']} #{$LDFLAGS} -L#{RbConfig::CONFIG['libdir']}".gsub("$(ldflags)", "").gsub("-fno-common", "")
$CXXFLAGS = " -std=gnu++98 #{$CFLAGS}"
$CPPFLAGS = $ARCH_FLAG = $DLDFLAGS = ""

GMAKE_CMD = Config::CONFIG['host_os'].downcase =~ /bsd|solaris/ ? "gmake" : "make"
TAR_CMD = SOLARIS_32 ? 'gtar' : 'tar'
PATCH_CMD = SOLARIS_32 ? 'gpatch' : 'patch'

if ENV['DEBUG']
  puts "Setting debug flags."
  $CFLAGS << " -O0 -ggdb -DHAVE_DEBUG"
  $EXTRA_CONF = ""
end

def patch(prefix, reason)
  puts "Patching libmemcached source for #{reason}."
  puts(cmd = "#{PATCH_CMD} -p1 -f < #{prefix}.patch")
  raise "'#{cmd}' failed" unless system(cmd) or ENV['DEV']
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
    if File.exist?("lib")
      puts "Libmemcached already built; run 'rake clean' first if you need to rebuild."
    else
      # have_sasl check may fail on OSX, skip it
      # unless RUBY_PLATFORM =~ /darwin/ or have_library('sasl2')
      #   raise "SASL2 not found. You need the libsasl2-dev library, which should be provided through your system's package manager."
      # end

      system("rm -rf #{BUNDLE_PATH}") unless ENV['DEBUG'] or ENV['DEV']

      puts "Building libmemcached."
      puts(cmd = "#{TAR_CMD} xzf #{BUNDLE} 2>&1")
      raise "'#{cmd}' failed" unless system(cmd)

      patch("libmemcached", "mark-dead behavior")
      patch("sasl", "SASL")
      patch("libmemcached-2", "get_from_last method")
      patch("libmemcached-3", "no block prepend and append")
      patch("libmemcached-4", "noop hash")
      patch("libmemcached-5", "get_len method")
      patch("libmemcached-6", "failure count bug")

      puts "Touching aclocal.m4  in libmemcached."
      puts(cmd = "touch -r #{BUNDLE_PATH}/m4/visibility.m4 #{BUNDLE_PATH}/configure.ac #{BUNDLE_PATH}/m4/pandora_have_sasl.m4")
      raise "'#{cmd}' failed" unless system(cmd)

      Dir.chdir(BUNDLE_PATH) do
        puts(cmd = "env CFLAGS='-fPIC #{$CFLAGS}' LDFLAGS='-fPIC #{$LDFLAGS}' ./configure --prefix=#{HERE} --without-memcached --disable-shared --disable-utils --disable-dependency-tracking #{$EXTRA_CONF} 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)

        #Running the make command in another script invoked by another shell command solves the "cd ." issue on FreeBSD 6+
        system("GMAKE_CMD='#{GMAKE_CMD}' CXXFLAGS='#{$CXXFLAGS}' SOURCE_DIR='#{BUNDLE_PATH}' HERE='#{HERE}' ruby ../extconf-make.rb")
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

check_libmemcached

if ENV['SWIG']
  puts "Running SWIG."
  puts(cmd = "swig #{$defines} #{$includes} -ruby -autorename rlibmemcached.i")
  raise "'#{cmd}' failed" unless system(cmd)
  puts(cmd = "sed -i '' 's/STR2CSTR/StringValuePtr/' rlibmemcached_wrap.c")
  raise "'#{cmd}' failed" unless system(cmd)
end

$CFLAGS << " -Os"
create_makefile 'rlibmemcached'
