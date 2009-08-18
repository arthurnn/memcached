require 'mkmf'

HERE = File.expand_path(File.dirname(__FILE__))
BUNDLE = Dir.glob("libmemcached-*.tar.gz").first
BUNDLE_PATH = BUNDLE.sub(".tar.gz", "")

DARWIN = `uname -sp` == "Darwin i386\n"

# is there a better way to do this?
if ENV['ARCHFLAGS']
  archflags = ENV['ARCHFLAGS']
elsif Config::CONFIG['host_os'] == 'darwin10.0'
  archflags = "-arch i386 -arch x86_64"
elsif Config::CONFIG['host_os'] =~ /darwin/
  archflags = "-arch i386 -arch ppc"
else
  archflags = ''
end

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
        
        cflags = "-fPIC"
        cxxflags = cflags
        ldflags = "-fPIC"
        extraconf = ''
        
        # again... is there a better way to do this?
        if DARWIN
          cflags = "#{cflags} #{archflags}"
          cxxflags = "-std=gnu++98 #{cflags}"
          ldflags = "#{ldflags} #{archflags}"
          extraconf = '--enable-dtrace --disable-dependency-tracking'
        end
        
        puts(cmd = "env CFLAGS='#{cflags}' LDFLAGS='#{ldflags}' ./configure --prefix=#{HERE} --without-memcached --disable-shared --disable-utils #{extraconf} 2>&1")
        raise "'#{cmd}' failed" unless system(cmd)
        puts(cmd = "make CXXFLAGS='#{cxxflags}' || true 2>&1")
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
  $CFLAGS << " #{archflags}"
  $LDFLAGS.gsub! /-arch \S+/, ''
  $LDFLAGS << " #{archflags}"
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
