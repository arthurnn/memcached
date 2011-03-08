
require "#{File.dirname(__FILE__)}/../setup"

$LOAD_PATH << "#{File.dirname(__FILE__)}/../../lib/"
require 'memcached'
require 'rubygems'
require 'ostruct'

GC.copy_on_write_friendly = true if GC.respond_to?("copy_on_write_friendly=")

class Worker
  def initialize(method_name, iterations, with_memory = 'false')
    @method = method_name || 'mixed'
    @i = (iterations || 10000).to_i
    @with_memory = with_memory

    puts "*** Running #{@method.inspect} test for #{@i} loops. ***"

    @key1 = "key1--------------------------------"
    @key2 = "key2--------------------------------"
    @key3 = "key3--------------------------------"

    @value = OpenStruct.new(
      :a => 1, :b => 2, "array" => [1, 2, 3], "hash" => {:badger => 1}
    )
    @marshalled = Marshal.dump(@value)

    @opts = [
      ['localhost:43042', 'localhost:43043', 'localhost:43044'],
      {
        :buffer_requests => true,
        :no_block => true,
        :noreply => false,
        :namespace => "namespace"
      }
    ]
    @cache = Memcached::Rails.new(*@opts)

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
          @cache.get @key2
        end
      when "get-increasing"
        one_k = "x"*1024
        @i.times do |i|
          @cache.set @key1, one_k*(i+1), 0, false
          @cache.get @key1, false
        end
      when "get-miss-increasing"
        @i.times do |i|
          @cache.delete @key2
          @cache.get @key2
        end
      when "add"
        @i.times do
          @cache.delete @key1
          @cache.add @key1, @value
        end
      when "add-present"
        @cache.set @key1, @value
        @i.times do
          @cache.add @key1, @value
        end
      when "mixed"
        @i.times do
          @cache.set @key1, @value
          @cache.set @key2, @value
          @cache.get @key1
          @cache.get @key3
          @cache.get [@key1, @key2, @key3]
          @cache.prepend @key1, @marshalled
          @cache.prepend @key2, @marshalled
          @cache.delete @key1
          @cache.delete @key2
        end
      when "everything"
        @i.times do
          @cache.set @key1, @value
          @cache.set @key2, @value
          @cache.get @key1
          @cache.get @key3
          @cache.get [@key1, @key2, @key3]
          @cache.prepend @key1, @marshalled
          @cache.prepend @key2, @marshalled
          @cache.delete @key1
          @cache.delete @key2
          @cache.prepend @key1, @marshalled
          @cache.prepend @key2, @marshalled
          @cache.get @key1
          @cache.get @key3
          cache = Memcached::Rails.new(*@opts)
          cache = @cache.clone
          servers = @cache.servers
          server = @cache.server_by_key(@key1)
          @cache.stats
        end
        @i.times do
          @cache.reset
        end
        system("killall -9 memcached")
        @i.times do
          @cache.set @key1, @value rescue nil
          @cache.get @key3 rescue nil
          @cache.delete @key2 rescue nil
        end
      when "stats"
        @cache.stats
      when "multiget"
        @i.times do
          @cache.get([@key1, @key2, @key3])
        end
      when "clone"
        @i.times do
          cache = @cache.clone
        end
      when "reset"
        @i.times do
          @cache.reset
        end
      when "servers"
        @i.times do
          @cache.servers
        end
      when "server_by_key"
        @i.times do
          @cache.server_by_key(@key1)
        end
      when "server_error"
        @i.times do
          begin
            @cache.set @key1, "I'm big" * 1000000
          rescue
          end
        end
      else
        raise "No such method"
    end

    @cache = nil

    if @with_memory == "true"
      puts "*** Garbage collect. ***"
      10.times do
        GC.start
        sleep 0.1
      end

      sts, server_sts, clients = 0, 0, 0
      ObjectSpace.each_object(Memcached) { clients += 1 }
      ObjectSpace.each_object(Rlibmemcached::MemcachedSt) { sts += 1 }
      ObjectSpace.each_object(Rlibmemcached::MemcachedServerSt) { server_sts += 1 }
      puts "*** memcached_st structs: #{sts} ***"
      puts "*** memcached_server_st structs: #{server_sts} ***"
      puts "*** Memcached instances: #{clients} ***"

      begin;
        require 'memory'
        Process.memory.each { |key, value| puts "#{key}: #{value/1024.0}M" }
      rescue LoadError
      end
    end
  end
end
