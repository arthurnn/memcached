
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"
UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached') 

require 'memcached'
require 'benchmark'
require 'rubygems'
require 'ruby-debug' if ENV['DEBUG']
begin; require 'memory'; rescue LoadError; end

puts `uname -a`
puts "Ruby #{RUBY_VERSION}p#{RUBY_PATCHLEVEL}"

[ ["memcached", "memcached"]], 
  ["binary42-remix-stash", "remix-stash"]], 
  # ["astro-remcached", "remcached"], # Clobbers the "Memcached" constant
  ["memcache-client", "memcache"]].each do |gem_name, requirement|
  require requirement
  gem gem_name
  puts "Loaded #{$1} #{Gem.loaded_specs[gem_name].version.to_s rescue nil}"
end

class Bench

  def initialize(loops, recursion = 0)
    @loops = loops
    @recursion = recursion
    puts "Recursion level is #{@recursion}"
    
    # We'll use a simple @value to try to avoid spending time in Marshal,
    # which is a constant penalty that both clients have to pay
    @value = []
    @marshalled = Marshal.dump(@value)
    
    @ccache_networked = Memcached.new(
      ['127.0.0.1:43042', '127.0.0.1:43043'],
      {:buffer_requests => false, :no_block => false, :namespace => "namespace"})
    @ccache_noblock_networked = Memcached.new(
      ['127.0.0.1:43042', '127.0.0.1:43043'],
      {:no_block => true, :buffer_requests => true, :namespace => "namespace"})
    @ccache_unix = Memcached.new(
      ["#{UNIX_SOCKET_NAME}0","#{UNIX_SOCKET_NAME}1"],
      {:buffer_requests => false, :no_block => false, :namespace => "namespace"})
    @ccache_networked_binary = Memcached.new(
      ['127.0.0.1:43042', '127.0.0.1:43043'],
      {:buffer_requests => false, :no_block => false, :namespace => "namespace", :binary_protocol => true})
    @ccache_noblock_networked_binary = Memcached.new(
      ['127.0.0.1:43042', '127.0.0.1:43043'],
      {:no_block => true, :buffer_requests => true, :namespace => "namespace", :binary_protocol => true})
      
    @rcache = MemCache.new(
      ['127.0.0.1:43042', '127.0.0.1:43043']
      {:namespace => "namespace"})
      
    # API doesn't let you set servers...what?
    Remix::Stash.class_variable_get("@@clusters")[:default] = 
      Remix::Stash::Cluster.new(['127.0.0.1:43042', '127.0.0.1:43043'])
    @rstash = Remix::Stash.new(:root)
    
    @ecache = Remcached

    
    @key1 = "Short"
    @key2 = "Sym1-2-3::45"*8
    @key3 = "Long"*40
    @key4 = "Medium"*8
    @key5 = "Medium2"*8 
    @key6 = "Long3"*40 
    
    system("ruby #{HERE}/../setup.rb")
    sleep(1)
  end

  def run(level = @recursion)
    level > 0 ? run(level - 1) : benchmark
  end
  
  private
  
  def benchmark(clients, name)
    clients.each do |client|
    
    end
  end
  
  def benchmark    
    Benchmark.bm(35) do |x|
    
      if defined? Memcached
        @m = Memcached.new(
        @opts_networked[0],
        @opts_networked[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("set:plain:noblock:memcached:net:bin") do
          @loops.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
          end
        end
        @m = Memcached.new(
        @opt_unix[0],
        @opt_unix[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("set:plain:noblock:memcached:uds") do
          @loops.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
          end
        end
        @m = Memcached.new(*@opts_networked)
        x.report("set:plain:memcached:net:bin") do
          @loops.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
          end
        end # if false
        @m = Memcached.new(*@opt_unix)
        x.report("set:plain:memcached:uds") do
          @loops.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("set:plain:memcache-client") do
          @loops.times do
            @m.set @key1, @marshalled, 0, true
            @m.set @key2, @marshalled, 0, true
            @m.set @key3, @marshalled, 0, true
          end
        end
      end
    
      if defined? Memcached
        @m = Memcached.new(
        @opts_networked[0],
        @opts_networked[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("set:ruby:noblock:memcached:net:bin") do
          @loops.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end
        @m = Memcached.new(
        @opt_unix[0],
        @opt_unix[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("set:ruby:noblock:memcached:uds") do
          @loops.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end
        @m = Memcached.new(*@opts_networked)
        x.report("set:ruby:memcached:net:bin") do
          @loops.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end # if false
        @m = Memcached.new(*@opt_unix)
        x.report("set:ruby:memcached:uds") do
          @loops.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("set:ruby:memcache-client") do
          @loops.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end
      end
    
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("get:plain:memcached:net:bin") do
          @loops.times do
            @m.get @key1, false
            @m.get @key2, false
            @m.get @key3, false
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("get:plain:memcached:uds") do
          @loops.times do
            @m.get @key1, false
            @m.get @key2, false
            @m.get @key3, false
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("get:plain:memcache-client") do
          @loops.times do
            @m.get @key1, true
            @m.get @key2, true
            @m.get @key3, true
          end
        end
      end
    
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("get:ruby:memcached:net:bin") do
          @loops.times do
            @m.get @key1
            @m.get @key2
            @m.get @key3
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("get:ruby:memcached:uds") do
          @loops.times do
            @m.get @key1
            @m.get @key2
            @m.get @key3
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("get:ruby:memcache-client") do
          @loops.times do
            @m.get @key1
            @m.get @key2
            @m.get @key3
          end
        end
      end

      if defined? Memcached
        @m = Memcached.new(*@opts_networked)

        # Avoid rebuilding the array every request
        keys = [@key1, @key2, @key3, @key4, @key5, @key6]

        x.report("multiget:ruby:memcached:net:bin") do
          @loops.times do
            @m.get keys
          end
        end
        @m = Memcached.new(*@opt_unix)

        # Avoid rebuilding the array every request
        keys = [@key1, @key2, @key3, @key4, @key5, @key6]

        x.report("multiget:ruby:memcached:uds") do
          @loops.times do
            @m.get keys
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("multiget:ruby:memcache-client") do
          @loops.times do
            # We don't use the keys array because splat is slow
            @m.get_multi @key1, @key2, @key3, @key4, @key5, @key6
          end
        end
      end
    
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("get-miss:ruby:memcached:net:bin") do
          @loops.times do
            begin @m.delete @key1; rescue Memcached::NotFound; end
            begin @m.delete @key2; rescue Memcached::NotFound; end
            begin @m.delete @key3; rescue Memcached::NotFound; end
            begin @m.get @key1; rescue Memcached::NotFound; end
            begin @m.get @key2; rescue Memcached::NotFound; end
            begin @m.get @key3; rescue Memcached::NotFound; end
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("get-miss:ruby:memcached:uds") do
          @loops.times do
            begin @m.delete @key1; rescue Memcached::NotFound; end
            begin @m.delete @key2; rescue Memcached::NotFound; end
            begin @m.delete @key3; rescue Memcached::NotFound; end
            begin @m.get @key1; rescue Memcached::NotFound; end
            begin @m.get @key2; rescue Memcached::NotFound; end
            begin @m.get @key3; rescue Memcached::NotFound; end
          end
        end
      end      

      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("get-miss:ruby:memcache-client") do
          @loops.times do
            begin @m.delete @key1; rescue; end
            begin @m.delete @key2; rescue; end
            begin @m.delete @key3; rescue; end
            begin @m.get @key1; rescue; end
            begin @m.get @key2; rescue; end
            begin @m.get @key3; rescue; end
          end
        end
      end
      
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("append-miss:ruby:memcached:net:bin") do
          @loops.times do
            begin @m.delete @key1; rescue Memcached::NotFound; end
            begin @m.delete @key2; rescue Memcached::NotFound; end
            begin @m.delete @key3; rescue Memcached::NotFound; end
            begin @m.append @key1, @value; rescue Memcached::NotStored; end
            begin @m.append @key2, @value; rescue Memcached::NotStored; end
            begin @m.append @key3, @value; rescue Memcached::NotStored; end
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("append-miss:ruby:memcached:uds") do
          @loops.times do
            begin @m.delete @key1; rescue Memcached::NotFound; end
            begin @m.delete @key2; rescue Memcached::NotFound; end
            begin @m.delete @key3; rescue Memcached::NotFound; end
            begin @m.append @key1, @value; rescue Memcached::NotStored; end
            begin @m.append @key2, @value; rescue Memcached::NotStored; end
            begin @m.append @key3, @value; rescue Memcached::NotStored; end
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("append-miss:ruby:memcache-client") do
          @loops.times do
            begin @m.delete @key1; rescue; end
            begin @m.delete @key2; rescue; end
            begin @m.delete @key3; rescue; end
            begin @m.append @key1, @value; rescue; end
            begin @m.append @key2, @value; rescue; end
            begin @m.append @key3, @value; rescue; end
          end
        end
      end
                      
      if defined? Memcached
        @m = Memcached.new(
        @opts_networked[0],
        @opts_networked[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("mixed:ruby:noblock:memcached:net:bin") do
          @loops.times do
            @m.set @key1, @value
            @m.get @key1
            @m.set @key2, @value
            @m.get @key2
            @m.set @key3, @value
            @m.get @key3
          end
        end
        @m = Memcached.new(
        @opt_unix[0],
        @opt_unix[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("mixed:ruby:noblock:memcached:uds") do
          @loops.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.get @key1
            @m.get @key2
            @m.get @key3
          end
        end
        @m = Memcached.new(*@opts_networked)
        x.report("mixed:ruby:memcached:net:bin") do
          @loops.times do
            @m.set @key1, @value
            @m.get @key1
            @m.set @key2, @value
            @m.get @key2
            @m.set @key3, @value
            @m.get @key3
          end
        end # if false
        @m = Memcached.new(*@opt_unix)
        x.report("mixed:ruby:memcached:uds") do
          @loops.times do
            @m.set @key1, @value
            @m.get @key1
            @m.set @key2, @value
            @m.get @key2
            @m.set @key3, @value
            @m.get @key3
          end
        end # if false
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("mixed:ruby:memcache-client") do
          @loops.times do
            @m.set @key1, @value
            @m.get @key1
            @m.set @key2, @value
            @m.get @key2
            @m.set @key3, @value
            @m.get @key3
          end
        end
      end
    
      if defined? Memcached
        unless ARGV.include? "--no-hash"
          Memcached::HASH_VALUES.each do |mode, int|
            @m = Memcached.new(@opt_unix[0], @opt_unix[1].merge(:hash => mode))
            x.report("hash:#{mode}") do
              @loops.times do
                Rlibmemcached.memcached_generate_hash_rvalue(@key1, int)
                Rlibmemcached.memcached_generate_hash_rvalue(@key2, int)
                Rlibmemcached.memcached_generate_hash_rvalue(@key3, int)
                Rlibmemcached.memcached_generate_hash_rvalue(@key4, int)
                Rlibmemcached.memcached_generate_hash_rvalue(@key5, int)
                Rlibmemcached.memcached_generate_hash_rvalue(@key6, int)
              end
            end
          end
        end
      end

    end    
  end
end

Bench.new((ENV["LOOPS"] || 10000).to_i, (ENV["RECURSION"] || 0).to_i).run

Process.memory.each do |key, value|
  puts "#{key}: #{value/1024.0}M"
end if Process.respond_to? :memory
