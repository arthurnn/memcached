
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
      when "delete"
        @i.times do
          @cache.set @key1, @value
          @cache.delete @key1
        end
      when "delete-miss"
        @i.times do
          @cache.delete @key1
        end
      when "get-miss"
        @i.times do
          begin
            @cache.get @key2
          rescue Memcached::NotFound
          end
        end
      when "get-random"
        @i.times do |i|
          key = "#{@key1}_#{'x'*i}"
          @cache.set key, @value
          @cache.get key
        end
      when "get-miss-random"
        @i.times do |i|
          key = "#{@key1}_#{'x'*i}"
          begin
            @cache.get key
          rescue Memcached::NotFound
          end
        end
      when "add"
        @i.times do
          begin
            @cache.delete @key1
          rescue
          end
          @cache.add @key1, @value
        end
      when "add-present"
        @cache.set @key1, @value
        @i.times do
          begin
            @cache.add @key1, @value
          rescue Memcached::NotStored
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
          cache = @cache.clone
          cache.destroy(false)
        end
      when "clone-nodestroy"
        @i.times do
          @cache.clone      
        end
      when "servers"
        @i.times do
          @cache.servers
        end
      else
        raise "No such method"
    end
  end  
end

Worker.new(ENV['METHOD'], ENV['LOOPS']).work
