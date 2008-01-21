
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'memcached'
require 'benchmark'

@value = []
@marshalled = Marshal.dump(@value)

@opts = [
  ['127.0.0.1:43042', '127.0.0.1:43043'], 
  {
    :buffer_requests => true,
    :no_block => true,
    :namespace => "benchmark_namespace"
  }
]
@key1 = "Short" 
@key2 = "Sym1-2-3::45"*8
@key3 = "Long"*40
@key4 = "Medium"*8

def restart_servers
  system("ruby #{HERE}/../setup.rb")
  sleep(1)
end

Benchmark.bm(31) do |x|
  n = 3000
  restart_servers 

  @m = Memcached.new(*@opts)
  x.report("set:ruby:memcached") do
    n.times do
      @m.set @key1, @value
      @m.set @key2, @value
      @m.set @key3, @value
      @m.set @key1, @value
      @m.set @key2, @value
      @m.set @key3, @value
    end
  end

  @m = Memcached.new(*@opts)
  x.report("get:ruby:memcached") do
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
