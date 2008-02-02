
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'memcached'

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

@key = "key-"*8

system("ruby #{HERE}/../setup.rb")
sleep(1)

@m = Memcached.new(*@opts)

1000.times do
  @m.set @key, @value
  @m.set @key, @value
  @m.set @key, @value
  @m.get @key
  @m.get @key
  @m.get @key
  @m.set @key, @value
  @m.get @key
  @m.set @key, @value
  @m.get @key
  @m.set @key, @value
  @m.get @key
end
