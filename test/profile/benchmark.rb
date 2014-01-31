Bundler.require(:benchmark)

HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"
UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached')
JRUBY = defined?(JRUBY_VERSION)

require 'ffi/times' if JRUBY
require 'memcached'
require 'benchmark'
require 'rubygems'
require 'ruby-debug' if ENV['DEBUG'] && !JRUBY
if ENV['PROFILE']
  require 'ruby-prof'
  require 'fileutils'
  FileUtils.mkdir_p 'profiles'
end
begin; require 'memory'; rescue LoadError; end


puts `uname -a`
puts `ruby -v`
puts `env | egrep '^RUBY'`
puts "Ruby #{RUBY_VERSION}p#{RUBY_PATCHLEVEL}"

[
  ["memcached"],
  ["remix-stash", "remix/stash"],
  [JRUBY ? "jruby-memcache-client" : "memcache-client", "memcache"],
  ["kgio"], ["dalli"]
].each do |gem_name, requirement|
  begin
    require requirement || gem_name
    gem gem_name
    puts "Loaded #{gem_name} #{Gem.loaded_specs[gem_name].version.to_s rescue nil}"
  rescue LoadError
  end
end

class Remix::Stash
  # Remix::Stash API doesn't let you set servers
  @@clusters = {:default => Remix::Stash::Cluster.new(['127.0.0.1:43042', '127.0.0.1:43043'])}
end

class Dalli::ClientCompat < Dalli::Client
  def set(*args)
    super(*args[0..2])
  end
  def get(*args)
    super(args.first)
  end
  def get_multi(*args)
    super(args.first)
  end
  def append(*args)
    super
  rescue Dalli::DalliError
  end
  def prepend(*args)
    super
  rescue Dalli::DalliError
  end
  def exist?(key)
    !get(key).nil?
  end
end

class Bench

  def initialize(loops = nil, stack_depth = nil)
    @loops = (loops || 50000).to_i
    @stack_depth = (stack_depth || 0).to_i

    puts "PID is #{Process.pid}"
    puts "Loops is #{@loops}"
    puts "Stack depth is #{@stack_depth}"

    @m_value = Marshal.dump(
      @small_value = ["testing"])
    @m_large_value = Marshal.dump(
      @large_value = [{"test" => "1", "test2" => "2", Object.new => "3", 4 => 4, "test5" => 2**65}] * 2048)

    puts "Small value size is: #{@m_value.size} bytes"
    puts "Large value size is: #{@m_large_value.size} bytes"

    @keys = [
      @k1 = "Short",
      @k2 = "Sym1-2-3::45" * 8,
      @k3 = "Long" * 40,
      @k4 = "Medium" * 8,
      @k5 = "Medium2" * 8,
      @k6 = "Long3" * 40]

    reset_servers
    reset_clients

    Benchmark.bm(36) do |x|
      @benchmark = x
    end
  end

  def run(level = @stack_depth)
    level > 0 ? run(level - 1) : run_without_recursion
  end

  private

  def reset_servers
    system("ruby #{HERE}/../setup.rb")
    sleep(1)
  end

  def reset_clients
    # Other clients
    @clients = {
      "mclient:ascii" => MemCache.new(['127.0.0.1:43042', '127.0.0.1:43043']),
      "stash:bin" => Remix::Stash.new(:root),
      "dalli:bin" => Dalli::ClientCompat.new(['127.0.0.1:43042', '127.0.0.1:43043'], :marshal => false, :threadsafe => false)}

    # Us
    @clients.merge!({
      "libm:ascii" => Memcached::Rails.new(
        ['127.0.0.1:43042', '127.0.0.1:43043'],
        :buffer_requests => false, :no_block => false, :namespace => "namespace"),
      "libm:ascii:pipeline" => Memcached::Rails.new(
        ['127.0.0.1:43042', '127.0.0.1:43043'],
        :no_block => true, :buffer_requests => true, :noreply => true, :namespace => "namespace"),
      "libm:ascii:udp" => Memcached::Rails.new(
        ["#{UNIX_SOCKET_NAME}0", "#{UNIX_SOCKET_NAME}1"],
        :buffer_requests => false, :no_block => false, :namespace => "namespace"),
      "libm:bin" => Memcached::Rails.new(
        ['127.0.0.1:43042', '127.0.0.1:43043'],
        :buffer_requests => false, :no_block => false, :namespace => "namespace", :binary_protocol => true),
      "libm:bin:buffer" => Memcached::Rails.new(
        ['127.0.0.1:43042', '127.0.0.1:43043'],
        :no_block => true, :buffer_requests => true, :namespace => "namespace", :binary_protocol => true)})
  end

  def benchmark_clients(test_name, populate_keys = true)
    return if ENV["TEST"] and !test_name.include?(ENV["TEST"])

    @clients.keys.sort.each do |client_name|
      next if ENV["CLIENT"] and !client_name.include?(ENV["CLIENT"])
      next if client_name == "stash" and test_name == "set-large" # Don't let stash break the world

      client = @clients[client_name]
      begin
        if populate_keys
          client.set @k1, @m_value, 0, true
          client.set @k2, @m_value, 0, true
          client.set @k3, @m_value, 0, true
        else
          client.delete @k1
          client.delete @k2
          client.delete @k3
        end

        # Force any JITs to run
        10003.times { yield client }

        GC.disable if !JRUBY
        RubyProf.start if ENV['PROFILE']
        @benchmark.report("#{test_name}: #{client_name}") { @loops.times { yield client } }
        if ENV['PROFILE']
          prof = RubyProf::MultiPrinter.new(RubyProf.stop)
          prof.print(:path => 'profiles', :profile => "#{test_name}-#{client_name.gsub(':','-')}")
        end
      rescue Exception => e
        puts "#{test_name}: #{client_name} => #{e.inspect}" if ENV["DEBUG"]
        reset_clients
      end
      GC.enable if !JRUBY
    end
    puts
  end

  def benchmark_hashes(hashes, test_name)
    hashes.each do |hash_name, int|
      @m = Memcached::Rails.new(:hash => hash_name)
      @benchmark.report("#{test_name}:#{hash_name}") do
        (@loops * 5).times { yield int }
      end
    end
  end

  def run_without_recursion
    benchmark_clients("set") do |c|
      c.set @k1, @m_value, 0, true
      c.set @k2, @m_value, 0, true
      c.set @k3, @m_value, 0, true
    end

    benchmark_clients("get") do |c|
      c.get @k1, true
      c.get @k2, true
      c.get @k3, true
    end

    benchmark_clients("get-multi") do |c|
      c.get_multi @keys, true
    end

    benchmark_clients("append") do |c|
      c.append @k1, @m_value
      c.append @k2, @m_value
      c.append @k3, @m_value
    end

    benchmark_clients("prepend") do |c|
      c.prepend @k1, @m_value
      c.prepend @k2, @m_value
      c.prepend @k3, @m_value
    end

    benchmark_clients("delete") do |c|
      c.delete @k1
      c.delete @k2
      c.delete @k3
    end

    benchmark_clients("exist") do |c|
      c.exist? @k1
      c.exist? @k2
      c.exist? @k3
    end

    benchmark_clients("get-missing", false) do |c|
      c.get @k1
      c.get @k2
      c.get @k3
    end

    benchmark_clients("append-missing", false) do |c|
      c.append @k1, @m_value
      c.append @k2, @m_value
      c.append @k3, @m_value
    end

    benchmark_clients("prepend-missing", false) do |c|
      c.prepend @k1, @m_value
      c.prepend @k2, @m_value
      c.prepend @k3, @m_value
    end

    benchmark_clients("exist-missing", false) do |c|
      c.exist? @k1
      c.exist? @k2
      c.exist? @k3
    end

    benchmark_clients("set-large") do |c|
      c.set @k1, @m_large_value, 0, true
      c.set @k2, @m_large_value, 0, true
      c.set @k3, @m_large_value, 0, true
    end

    benchmark_clients("get-large") do |c|
      c.get @k1, true
      c.get @k2, true
      c.get @k3, true
    end

    if defined?(Memcached) && !ENV['TEST'] && !ENV['CLIENT']
      benchmark_hashes(Memcached::HASH_VALUES, "hash") do |i|
        Rlibmemcached.memcached_generate_hash_rvalue(@k1, i)
        Rlibmemcached.memcached_generate_hash_rvalue(@k2, i)
        Rlibmemcached.memcached_generate_hash_rvalue(@k3, i)
        Rlibmemcached.memcached_generate_hash_rvalue(@k4, i)
        Rlibmemcached.memcached_generate_hash_rvalue(@k5, i)
        Rlibmemcached.memcached_generate_hash_rvalue(@k6, i)
      end
    end
  end
end

Bench.new(ENV["LOOPS"], ENV["STACK_DEPTH"]).run

Process.memory.each do |key, value|
  puts "#{key}: #{value/1024.0}M"
end if Process.respond_to? :memory
