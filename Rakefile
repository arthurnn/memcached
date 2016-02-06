require 'echoe'

ENV["GEM_CERTIFICATE_CHAIN"]="memcached.pem"

Echoe.new("memcached") do |p|
  p.author = "Evan Weaver"
  p.project = "evan"
  p.summary = "An interface to the libmemcached C client."
  p.licenses = "Academic Free License 3.0 (AFL-3.0)"
  p.rdoc_pattern = /README|TODO|LICENSE|CHANGELOG|BENCH|COMPAT|exceptions|experimental.rb|behaviors|rails.rb|memcached.rb/
  p.rdoc_options = %w[--line-numbers --inline-source --title Memcached --main README.rdoc --exclude=ext/bin --exclude=ext/libmemcached-.*/(clients|tests)]
  p.retain_gemspec = true
  p.development_dependencies = ["rake", "mocha", "echoe", "activesupport"]
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

task :swig do
  run("swig -DLIBMEMCACHED_WITH_SASL_SUPPORT -Iext/libmemcached-0.32 -ruby -autorename -o ext/rlibmemcached_wrap.c.in ext/rlibmemcached.i", "Running SWIG")
  swig_patches = {
    "#ifndef RUBY_INIT_STACK" => "#ifdef __NEVER__" # Patching SWIG output for JRuby.
  }.map{|pair| "s/#{pair.join('/')}/"}.join(';')
  # sed has different syntax for inplace switch in BSD and GNU version, so using intermediate file
  run("sed '#{swig_patches}' ext/rlibmemcached_wrap.c.in > ext/rlibmemcached_wrap.c", "Apply patches to SWIG output")
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
