gem 'echoe', '>= 4.5.6'
require 'echoe'

Echoe.new("memcached") do |p|
  p.author = "Evan Weaver"
  p.project = "fauna"
  p.summary = "An interface to the libmemcached C client."
  p.rdoc_pattern = /README|TODO|LICENSE|CHANGELOG|BENCH|COMPAT|exceptions|experimental.rb|behaviors|rails.rb|memcached.rb/
  p.clean_pattern += ["ext/Makefile",
                      "ext/bin",
                      "ext/include",
                      "ext/lib",
                      "ext/share",
                      "ext/**/Makefile",
                      "ext/libmemcached-*/autom4te.cache",
                      "ext/libmemcached-*/clients/memcat",
                      "ext/libmemcached-*/clients/memcp",
                      "ext/libmemcached-*/clients/memdump",
                      "ext/libmemcached-*/clients/memerror",
                      "ext/libmemcached-*/clients/memflush",
                      "ext/libmemcached-*/clients/memrm",
                      "ext/libmemcached-*/clients/memslap",
                      "ext/libmemcached-*/clients/memstat",
                      "ext/libmemcached-*/tests/atomsmasher",
                      "ext/libmemcached-*/tests/startservers",
                      "ext/libmemcached-*/tests/testapp",
                      "ext/libmemcached-*/tests/testplus",
                      "ext/libmemcached-*/tests/udptest",
                      "ext/libmemcached-*/config.h",
                      "ext/libmemcached-*/config.log",
                      "ext/libmemcached-*/config.status",
                      "ext/libmemcached-*/docs/*.[1,3]",
                      "ext/libmemcached-*/libmemcached/memcached_configure.h",
                      "ext/libmemcached-*/libtool",
                      "ext/libmemcached-*/stamp*",
                      "ext/libmemcached-*/support/libmemcached.pc",
                      "ext/libmemcached-*/support/libmemcached-fc.spec",
                      "ext/libmemcached-*/**/*.s[oa]",
                      "ext/libmemcached-*/**/*.l[oa]",
                      "ext/conftest.dSYM",
                      "lib/rlibmemcached*"]
end

task :exceptions do
  $LOAD_PATH << "lib"
  require 'memcached'
  Memcached.constants.sort.each do |const_name|
    const = Memcached.send(:const_get, const_name)
    next if const == Memcached::Success or const == Memcached::Stored
    if const.is_a? Class and const < Memcached::Error
      puts "* Memcached::#{const_name}"
    end
  end
end

task :test_all do
  if !system("bash -c 'cd && source .bash_profile && rvm use ree-1.8.7-2010.02 && cd - && rake clean && rake'")
    puts "REE test failed"
    exit(1)
  end
  if !system("bash -c 'cd && source .bash_profile && rvm use 1.9.2-p180 && cd - && rake clean && rake'")
    puts "1.9 test failed"
    exit(1)
  end
end

task :prerelease => [:test_all]

task :benchmark do
 exec("ruby #{File.dirname(__FILE__)}/test/profile/benchmark.rb")
end

task :rb_profile do
 exec("ruby #{File.dirname(__FILE__)}/test/profile/rb_profiler.rb")
end

task :c_profile do
 exec("ruby #{File.dirname(__FILE__)}/test/profile/c_profiler.rb")
end

task :valgrind do
 exec("ruby #{File.dirname(__FILE__)}/test/profile/valgrind.rb")
end
