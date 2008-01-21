
HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'memcached'
require 'ostruct'
require 'benchmark'
require 'rubygems'
require 'ruby-prof'

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

system("ruby #{HERE}/../setup.rb")
sleep(1)

@m = Memcached.new(*@opts)

result = RubyProf.profile do  
  500.times do
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

printer = RubyProf::GraphPrinter.new(result)
printer.print(STDOUT, 0)
