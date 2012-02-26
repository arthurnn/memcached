gem 'echoe', '>= 4.5.6'
require 'echoe'

Echoe.new("memcached") do |p|
  p.author = "Evan Weaver"
  p.project = "evan"
  p.summary = "An interface to the libmemcached C client."
  p.rdoc_pattern = /README|TODO|LICENSE|CHANGELOG|BENCH|COMPAT|exceptions|experimental.rb|behaviors|rails.rb|memcached.rb/
  p.rdoc_options = %w[--line-numbers --inline-source --title Memcached --main README.rdoc --exclude=ext/bin --exclude=ext/libmemcached-.*/(clients|tests)]
  p.clean_pattern += ["ext/Makefile",
                      "ext/bin",
                      "ext/include",
                      "ext/lib",
                      "ext/share",
                      "ext/**/Makefile",
                      "ext/Makefile.in",
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
                      "lib/rlibmemcached*",
                      "**/*.rbc"]
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

def with_vms(cmd)
  ["ree-1.8.7-2011.03", "1.9.2", "rbx"].each do |vm| #, "jruby-head"
    if !system("bash -c 'cd && source .bash_profile && rvm use #{vm} && cd - && rake clean >> /dev/null && rake compile >> /dev/null && rake #{cmd}'")
      puts "#{vm} #{cmd} failed"
      exit(1)
    end
  end
end

task :test_all do
  with_vms("test")
end

task :prerelease => [:test_all]

task :benchmark_all do
  with_vms("benchmark CLIENT=libm")
end

