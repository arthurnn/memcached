
class Memcached

  FLAGS = 0x0

  DEFAULTS = {
    :hash => :default,
    :distribution => :consistent,
    :buffer_requests => false,
    :support_cas => false,
    :tcp_nodelay => false,
    :no_block => false
  }
  
  attr_reader :namespace
  attr_reader :options

  ### Configuration
  
  def initialize(servers, opts = {})
    @struct = Libmemcached::MemcachedSt.new
    Libmemcached.memcached_create(@struct)

    # Servers
    Array(servers).each_with_index do |server, index|
      unless server.is_a? String and server =~ /^(\d{1,3}\.){3}\d{1,3}:\d{1,5}$/
        raise ArgumentError, "Servers must be in the format ip:port (e.g., '127.0.0.1:11211')" 
      end
      host, port = server.split(":")
      Libmemcached.memcached_server_add(@struct, host, port.to_i)

      # XXX To be removed once Krow fixes the write_ptr bug
      Libmemcached.memcached_repair_server_st(@struct, 
        Libmemcached.memcached_select_server_at(@struct, index)
      )
    end  
    
    # Namespace
    @namespace = opts[:namespace]
    raise ArgumentError, "Invalid namespace" if namespace.to_s =~ / /

    # Behaviors
    @options = DEFAULTS.merge(opts)
    options.each do |option, value|
      set_behavior(option, value) unless option == :namespace
    end
  end

  def servers
    server_structs.map do |server|
      "#{server.hostname}:#{server.port}"
    end
  end
  
  def clone
    # XXX Could be more efficient if we used Libmemcached.memcached_clone(@struct)
    self.class.new(servers, options)
  end
  
  alias :dup :clone

  ### Configuration helpers

  private
    
  def server_structs
    array = []
    @struct.hosts.count.times do |i|
      array << Libmemcached.memcached_select_server_at(@struct, i)
    end
    array
  end    
    
  ### Operations
  
  public
  
  def set(key, value, timeout=0, marshal=true)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Libmemcached.memcached_set(@struct, ns(key), value, timeout, FLAGS)
    )
  end
  
  def get(key, marshal=true)
    if key.is_a? Array
      # Multi get
      # XXX Waiting on the real implementation
      key.map do |this_key|
        begin
          get(this_key, marshal)
        rescue NotFound
          # XXX Not sure how this behavior should be defined
        end
      end
    else
      # Single get
      # XXX Server doesn't validate. Possibly a performance problem.
      raise ClientError, "Invalid key" if !key.is_a? String or key =~ /\s/ 
        
      value, flags, ret = Libmemcached.memcached_get_ruby_string(@struct, ns(key))
      check_return_code(ret)
      value = Marshal.load(value) if marshal
      value
    end
  end  
  
  public
  
  def delete(key, timeout=0)
    check_return_code(
      Libmemcached.memcached_delete(@struct, ns(key), timeout)
    )  
  end
  
  def add(key, value, timeout=0, marshal=true)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Libmemcached.memcached_add(@struct, ns(key), value, timeout, FLAGS)
    )
  end
  
  def increment(key, offset=1)
    ret, value = Libmemcached.memcached_increment(@struct, ns(key), offset)
    check_return_code(ret)
    value
  end
  
  def decrement(key, offset=1)
    ret, value = Libmemcached.memcached_decrement(@struct, ns(key), offset)
    check_return_code(ret)
    value
  end
  
  alias :incr :increment
  alias :decr :decrement
  
  def replace
    raise NotImplemented
  end
  
  def append
    raise NotImplemented
  end
  
  def prepend
    raise NotImplemented
  end
  
  def cas
    raise "CAS not enabled" unless options[:support_cas]
    raise NotImplemented
  end
  
  def stats
    stats = Hash.new([])
    
    stat_struct, ret = Libmemcached.memcached_stat(@struct, "")
    check_return_code(ret)
    
    keys, ret = Libmemcached.memcached_stat_get_keys(@struct, stat_struct)
    check_return_code(ret)
    
    keys.each do |key|
       server_structs.size.times do |index|

         value, ret = Libmemcached.memcached_stat_get_value(
           @struct, 
           Libmemcached.memcached_select_stat_at(@struct, stat_struct, index),
           key)
         check_return_code(ret)

         value = case value
           when /^\d+\.\d+$/: value.to_f 
           when /^\d+$/: value.to_i
           else value
         end           
         
         stats[key.to_sym] += [value]
       end
    end
    
    Libmemcached.memcached_stat_free(@struct, stat_struct)
    stats
  end  
  
  ### Operations helpers
  
  private

  def ns(key)
    "#{@namespace}#{key}"
  end
    
  def check_return_code(ret)
    return true if ret == 0
    raise @@exceptions[ret]
  end  
    
end
