
# We don't care about the Ruby heap 
GC.disable 

HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'memcached'

class Worker  
  def initialize(method_name, iterations)
    @method = method_name || 'mixed'
    @i = (iterations || 1000).to_i

    @key1 = "key1-"*8  
    @key2 = "key2-"*8  
    
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
    system("ruby #{HERE}/../setup.rb")
    sleep(1)  
    @cache = Memcached.new(*@opts)

    @cache.set @key1, @value
  end
  
  def work
    case @method
      when "set"
        @i.times do
          @cache.set @key1, @value
        end
      when "get"
        @i.times do
          @cache.get @key1
        end
      when "miss"
        @i.times do
          begin
            @cache.get @key2
          rescue Memcached::NotFound
          end
        end
      when "mixed"
        @i.times do
          @cache.set @key1, @value
          @cache.get @key1
        end
      when "stats"
        @i.times do
          @cache.stats
        end
      when "multiget"
        @i.times do
          @cache.get([@key1, @key2])        
        end
      when "clone"
        @i.times do
          @cache.clone
        end
    end
  end  
end

Worker.new(ENV['METHOD'], ENV['LOOPS']).work
