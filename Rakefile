require 'echoe'

Echoe.new("memcached") do |p|
  p.author = "Evan Weaver"
  p.project = "fauna"
  p.summary = "An interface to the libmemcached C client."
  p.rdoc_pattern = /README|TODO|LICENSE|CHANGELOG|BENCH|COMPAT|exceptions|experimental.rb|behaviors|rails.rb|memcached.rb/
  p.clean_pattern += ["ext/lib", "ext/include", "ext/share", "ext/libmemcached-?.??", "ext/bin", "ext/conftest.dSYM", "lib/rlibmemcached.bundle.dSYM"]
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
  if !system("rvm use ree-1.8.7-2010.02 && rake clean && rake")
    puts "REE test failed"
    exit(1)
  end
  if !system("rvm use ruby-1.9.2 && rake clean && rake")
    puts "1.9 test failed"
    exit(1)
  end
end

task :release => [:test_all, :clean, :package]

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
