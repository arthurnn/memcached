
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"
UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached') 

require 'memcached'
require 'benchmark'
require 'rubygems'
require 'ruby-debug' if ENV['DEBUG']
begin; require 'memory'; rescue LoadError; end

ARGV << "--with-memcached" << "--with-memcache-client" if ARGV.join !~ /--with/

puts `uname -a`
puts "Ruby #{RUBY_VERSION}p#{RUBY_PATCHLEVEL}"

ARGV.each do |flag|
  require_name = flag[/--with-(\w+)/, 1]
  gem_name = flag[/--with-(.*)$/, 1]
  begin
    require require_name
    gem gem_name
    puts "Loaded #{$1} #{Gem.loaded_specs[gem_name].version.to_s rescue nil}"
  rescue LoadError
  end
end

recursion = ARGV.grep(/--recursion/).first[/(\d+)/, 1].to_i rescue 0

class Bench

  def initialize(recursion)

    @recursion = recursion
    puts "Recursion level is #{@recursion}"
    
    # We'll use a simple @value to try to avoid spending time in Marshal,
    # which is a constant penalty that both clients have to pay
    @value = []
    @marshalled = Marshal.dump(@value)
    
    @opts_networked = [
      ['127.0.0.1:43042', '127.0.0.1:43043'],
      {:buffer_requests => false, :no_block => false, :namespace => "namespace"}
    ]
    @opt_unix = [
      ["#{UNIX_SOCKET_NAME}0","#{UNIX_SOCKET_NAME}1"],
      {:buffer_requests => false, :no_block => false, :namespace => "namespace"}
    ]
    @key1 = "Short"
    @key2 = "Sym1-2-3::45"*8
    @key3 = "Long"*40
    @key4 = "Medium"*8
    # 5 and 6 are only used for multiget miss test
    @key5 = "Medium2"*8 
    @key6 = "Long3"*40 
    
    system("ruby #{HERE}/../setup.rb")
    sleep(1)
  end

  def run(level = @recursion)
    level > 0 ? run(level - 1) : benchmark
  end
  
  private
  
  def benchmark    
    Benchmark.bm(35) do |x|
    
      n = (ENV["LOOPS"] || 10000).to_i
    
      if defined? Memcached
        @m = Memcached.new(
        @opts_networked[0],
        @opts_networked[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("set:plain:noblock:memcached:net") do
          n.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
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
          n.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
          end
        end
        @m = Memcached.new(*@opts_networked)
        x.report("set:plain:memcached:net") do
          n.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
          end
        end # if false
        @m = Memcached.new(*@opt_unix)
        x.report("set:plain:memcached:uds") do
          n.times do
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
            @m.set @key1, @marshalled, 0, false
            @m.set @key2, @marshalled, 0, false
            @m.set @key3, @marshalled, 0, false
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("set:plain:memcache-client") do
          n.times do
            @m.set @key1, @marshalled, 0, true
            @m.set @key2, @marshalled, 0, true
            @m.set @key3, @marshalled, 0, true
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
        x.report("set:ruby:noblock:memcached:net") do
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
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
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end
        @m = Memcached.new(*@opts_networked)
        x.report("set:ruby:memcached:net") do
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end # if false
        @m = Memcached.new(*@opt_unix)
        x.report("set:ruby:memcached:uds") do
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("set:ruby:memcache-client") do
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
          end
        end
      end
    
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("get:plain:memcached:net") do
          n.times do
            @m.get @key1, false
            @m.get @key2, false
            @m.get @key3, false
            @m.get @key1, false
            @m.get @key2, false
            @m.get @key3, false
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("get:plain:memcached:uds") do
          n.times do
            @m.get @key1, false
            @m.get @key2, false
            @m.get @key3, false
            @m.get @key1, false
            @m.get @key2, false
            @m.get @key3, false
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("get:plain:memcache-client") do
          n.times do
            @m.get @key1, true
            @m.get @key2, true
            @m.get @key3, true
            @m.get @key1, true
            @m.get @key2, true
            @m.get @key3, true
          end
        end
      end
    
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("get:ruby:memcached:net") do
          n.times do
            @m.get @key1
            @m.get @key2
            @m.get @key3
            @m.get @key1
            @m.get @key2
            @m.get @key3
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("get:ruby:memcached:uds") do
          n.times do
            @m.get @key1
            @m.get @key2
            @m.get @key3
            @m.get @key1
            @m.get @key2
            @m.get @key3
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("get:ruby:memcache-client") do
          n.times do
            @m.get @key1
            @m.get @key2
            @m.get @key3
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

        x.report("multiget:ruby:memcached:net") do
          n.times do
            @m.get keys
          end
        end
        @m = Memcached.new(*@opt_unix)

        # Avoid rebuilding the array every request
        keys = [@key1, @key2, @key3, @key4, @key5, @key6]

        x.report("multiget:ruby:memcached:uds") do
          n.times do
            @m.get keys
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("multiget:ruby:memcache-client") do
          n.times do
            # We don't use the keys array because splat is slow
            @m.get_multi @key1, @key2, @key3, @key4, @key5, @key6
          end
        end
      end
    
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("missing:ruby:memcached:net") do
          n.times do
            begin @m.delete @key1; rescue Memcached::NotFound; end
            begin @m.get @key1; rescue Memcached::NotFound; end
            begin @m.delete @key2; rescue Memcached::NotFound; end
            begin @m.get @key2; rescue Memcached::NotFound; end
            begin @m.delete @key3; rescue Memcached::NotFound; end
            begin @m.get @key3; rescue Memcached::NotFound; end
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("missing:ruby:memcached:uds") do
          n.times do
            begin @m.delete @key1; rescue Memcached::NotFound; end
            begin @m.get @key1; rescue Memcached::NotFound; end
            begin @m.delete @key2; rescue Memcached::NotFound; end
            begin @m.get @key2; rescue Memcached::NotFound; end
            begin @m.delete @key3; rescue Memcached::NotFound; end
            begin @m.get @key3; rescue Memcached::NotFound; end
          end
        end
      end
      if defined? Memcached
        @m = Memcached.new(*@opts_networked)
        x.report("missing:ruby:memcached:inline") do
          n.times do
            @m.delete @key1 rescue nil
            @m.get @key1 rescue nil
            @m.delete @key2 rescue nil
            @m.get @key2 rescue nil
            @m.delete @key3 rescue nil
            @m.get @key3 rescue nil
          end
        end
        @m = Memcached.new(*@opt_unix)
        x.report("missing:ruby:memcached:uds:inline") do
          n.times do
            @m.delete @key1 rescue nil
            @m.get @key1 rescue nil
            @m.delete @key2 rescue nil
            @m.get @key2 rescue nil
            @m.delete @key3 rescue nil
            @m.get @key3 rescue nil
          end
        end
      end
      if defined? MemCache
        @m = MemCache.new(*@opts_networked)
        x.report("missing:ruby:memcache-client") do
          n.times do
            begin @m.delete @key1; rescue; end
            begin @m.get @key1; rescue; end
            begin @m.delete @key2; rescue; end
            begin @m.get @key2; rescue; end
            begin @m.delete @key3; rescue; end
            begin @m.get @key3; rescue; end
          end
        end
      end
          
      if defined? Memcached
        @m = Memcached.new(
        @opts_networked[0],
        @opts_networked[1].merge(:no_block => true, :buffer_requests => true)
        )
        x.report("mixed:ruby:noblock:memcached:net") do
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.get @key1
            @m.get @key2
            @m.get @key3
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
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.get @key1
            @m.get @key2
            @m.get @key3
            @m.set @key1, @value
            @m.get @key1
            @m.set @key2, @value
            @m.get @key2
            @m.set @key3, @value
            @m.get @key3
          end
        end
        @m = Memcached.new(*@opts_networked)
        x.report("mixed:ruby:memcached:net") do
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.get @key1
            @m.get @key2
            @m.get @key3
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
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.get @key1
            @m.get @key2
            @m.get @key3
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
          n.times do
            @m.set @key1, @value
            @m.set @key2, @value
            @m.set @key3, @value
            @m.get @key1
            @m.get @key2
            @m.get @key3
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
              n.times do
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

Bench.new(recursion).run

if Process.respond_to? :memory
  Process.memory.each do |key, value|
    puts "#{key}: #{value/1024.0}M"
  end
end
