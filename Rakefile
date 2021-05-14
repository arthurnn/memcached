require 'bundler/gem_tasks'
require 'rake/testtask'
require 'rake/extensiontask'

spec = Gem::Specification.load('memcached.gemspec')
Rake::ExtensionTask.new('rlibmemcached', spec) do |ext|
  ext.lib_dir = 'lib'
end

Rake::TestTask.new do |t|
  t.libs << 'lib' << 'test'
  t.pattern = 'test/**/*_test.rb'
  t.verbose = false
  t.warning = true
end
task :default => [:compile, :test]

Rake::Task[:clean].enhance do
  FileUtils.rm_rf(File.read(".gitignore").lines.flat_map { |l| Dir["**/#{l.chomp}"] })
end

task :swig do
  run("swig -DLIBMEMCACHED_WITH_SASL_SUPPORT -Ivendor/libmemcached-0.32 -ruby -autorename -o ext/rlibmemcached/rlibmemcached_wrap.c ext/rlibmemcached/rlibmemcached.i", "Running SWIG")
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

task :prerelease => [:manifest, :test, :install]

def run(cmd, reason)
  puts reason
  puts cmd
  raise "'#{cmd}' failed" unless system(cmd)
end
