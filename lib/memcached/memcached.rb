
=begin rdoc
The Memcached client class.
=end
class Memcached

  FLAGS = 0x0

  DEFAULTS = {
    :hash => :default,
    :distribution => :consistent,
    :buffer_requests => true,
    :no_block => true,
    :support_cas => false,
    :tcp_nodelay => false,
    :namespace => nil
  }
  
  IGNORED = 0 #:nodoc:
  
  attr_reader :options # Return the options Hash used to configure this instance.

###### Configuration

=begin rdoc      
Create a new Memcached instance. Accepts a single server string such as '127.0.0.1:11211', or an array of such strings, as well an an optional configuration hash.

Hostname lookups are not currently supported; you need to use the IP address.

Valid option parameters are:

<tt>:namespace</tt>:: A namespace string to prepend to every key.
<tt>:hash</tt>:: The name of a hash function to use. Possible values are: <tt>:crc</tt>, <tt>:default</tt>, <tt>:fnv1_32</tt>, <tt>:fnv1_64</tt>, <tt>:fnv1a_32</tt>, <tt>:fnv1a_64</tt>, <tt>:hsieh</tt>, <tt>:ketama</tt>, and <tt>:md5</tt>. <tt>:default</tt> is the fastest.
<tt>:distribution</tt>:: The type of distribution function to use. Possible values are <tt>:modula</tt> and <tt>:consistent</tt>. Note that this is decoupled from the choice of hash function.
<tt>:support_cas</tt>:: Flag CAS support in the client. Accepts <tt>true</tt> or <tt>false</tt>. Note that your server must also support CAS or you will trigger <b>Memcached::ProtocolError</b> exceptions.
<tt>:tcp_nodelay</tt>:: Turns on the no-delay feature for connecting sockets. Accepts <tt>true</tt> or <tt>false</tt>. Performance may or may not change, depending on your system.
<tt>:no_block</tt>:: Whether to use non-blocking, asynchronous IO for writes. Accepts <tt>true</tt> or <tt>false</tt>. 
<tt>:buffer_requests</tt>:: Whether to use an internal write buffer. Accepts <tt>true</tt> or <tt>false</tt>. Calling <tt>get</tt> or closing the connection will force the buffer to flush.

=end

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
    
    # Behaviors
    @options = DEFAULTS.merge(opts)
    options.each do |option, value|
      set_behavior(option, value) unless option == :namespace
    end

    # Namespace
    raise ArgumentError, "Invalid namespace" if options[:namespace].to_s =~ / /
    @namespace = options[:namespace]
  end

=begin rdoc
Return the array of server strings used to configure this instance.
=end
  def servers
    server_structs.map do |server|
      "#{server.hostname}:#{server.port}"
    end
  end

  # Safely copy this instance. Returns a Memcached instance. 
  #
  # <tt>clone</tt> is useful for threading, since each thread must have its own unshared Memcached object. 
  def clone
    # XXX Could be more efficient if we used Libmemcached.memcached_clone(@struct)
    self.class.new(servers, options)
  end
  
  #:stopdoc:
  alias :dup :clone #:nodoc:
  #:startdoc:

### Configuration helpers

  private
    
  # Return an array of raw <tt>memcached_host_st</tt> structs for this instance.
  def server_structs
    array = []
    @struct.hosts.count.times do |i|
      array << Libmemcached.memcached_select_server_at(@struct, i)
    end
    array
  end    
    
###### Operations
  
  public
  
### Setters
    
  # Set a key/value pair. Accepts a String <tt>key</tt> and an arbitrary Ruby object. Overwrites any existing value on the server.
  #
  # Accepts an optional <tt>timeout</tt> value to specify the maximum lifetime of the key on the server. <tt>timeout</tt> can be either an integer number of seconds, or a Time elapsed time object. (There is no guarantee that the key will persist as long as the <tt>timeout</tt>, but it will not persist longer.)
  #
  # Also accepts a <tt>marshal</tt> value, which defaults to <tt>true</tt>. Set <tt>marshal</tt> to <tt>false</tt> if you want the <tt>value</tt> to be set directly. 
  # 
  def set(key, value, timeout=0, marshal=true)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Libmemcached.memcached_set(@struct, ns(key), value, timeout, FLAGS)
    )
  end

  # Add a key/value pair. Raises <b>Memcached::NotStored</b> if the key already exists on the server. The parameters are the same as <tt>set</tt>.
  def add(key, value, timeout=0, marshal=true)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Libmemcached.memcached_add(@struct, ns(key), value, timeout, FLAGS)
    )
  end

  # Increment a key's value. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist. 
  #
  # Also accepts an optional <tt>offset</tt> paramater, which defaults to 1. <tt>offset</tt> must be an integer.
  #
  # Note that the key must be initialized to an unmarshalled integer first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>marshal</tt> set to <tt>false</tt>.
  def increment(key, offset=1)
    ret, value = Libmemcached.memcached_increment(@struct, ns(key), offset)
    check_return_code(ret)
    value
  end

  # Decrement a key's value. The parameters and exception behavior are the same as <tt>increment</tt>.
  def decrement(key, offset=1)
    ret, value = Libmemcached.memcached_decrement(@struct, ns(key), offset)
    check_return_code(ret)
    value
  end
  
  #:stopdoc:  
  alias :incr :increment
  alias :decr :decrement
  #:startdoc:

  # Replace a key/value pair. Raises <b>Memcached::NotFound</b> if the key does not exist on the server. The parameters are the same as <tt>set</tt>.
  def replace(key, value, timeout=0, marshal=true)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Libmemcached.memcached_replace(@struct, ns(key), value, timeout, FLAGS)
    )
  end

  # Appends a string to a key's value. Accepts a String <tt>key</tt> and a String <tt>value</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist on the server. 
  #
  # Note that the key must be initialized to an unmarshalled string first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>marshal</tt> set to <tt>false</tt>.
  def append(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Libmemcached.memcached_append(@struct, ns(key), value.to_s, IGNORED, FLAGS)
    )
  end
  
  # Prepends a string to a key's value. The parameters and exception behavior are the same as <tt>append</tt>.
  def prepend(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Libmemcached.memcached_prepend(@struct, ns(key), value.to_s, IGNORED, FLAGS)
    )
  end
  
  # Not yet implemented.
  #
  # Reads a key's value from the server and yields it to a block. Replaces the key's value with the result of the block as long as the key hasn't been updated in the meantime, otherwise raises <b>Memcached::NotStored</b>. Accepts a String <tt>key</tt> and a block.
  #
  # Also accepts an optional <tt>timeout</tt> value.
  #
  # CAS stands for "compare and swap", and avoids the need for manual key mutexing. CAS support must be enabled in Memcached.new or a <b>Memcached::ClientError</b> will be raised.
  def cas(*args)
    # Requires memcached HEAD
    raise NotImplemented
    raise ClientError, "CAS not enabled for this Memcached instance" unless options[:support_cas]
  end

### Deleters

  # Deletes a key/value pair from the server. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist.
  def delete(key)
    check_return_code(
      Libmemcached.memcached_delete(@struct, ns(key), IGNORED)
    )  
  end
  
### Getters  
  
  # Gets a key's value from the server. Accepts a String <tt>key</tt> or array of String <tt>keys</tt>.
  #
  # Also accepts a <tt>marshal</tt> value, which defaults to <tt>true</tt>. Set <tt>marshal</tt> to <tt>false</tt> if you want the <tt>value</tt> to be returned directly as a String. Otherwise it will be assumed to be a marshalled Ruby object and unmarshalled.
  #
  # If you pass a single key, and the key does not exist on the server, <b>Memcached::NotFound</b> will be raised. If you pass an array of keys, memcached's <tt>multiget</tt> mode will be used, and an array of values will be returned. Missing values in the array will be represented as instances of <b>Memcached::NotFound</b>. This behavior may change in the future.
  #
  def get(key, marshal=true)
    # XXX Could be faster if it didn't have to branch on the key type
    if key.is_a? Array
      # Multi get
      # XXX Waiting on the real implementation
      key.map do |this_key|
        begin
          get(this_key, marshal)
        rescue NotFound => e
          e
        end
      end
    else
      # Single get
      # XXX Server doesn't validate keys. Regex is possibly a performance problem.
      raise ClientError, "Invalid key" if key =~ /\s/ 
        
      value, flags, ret = Libmemcached.memcached_get_ruby_string(@struct, ns(key))
      check_return_code(ret)
      value = Marshal.load(value) if marshal
      value
    end
  end    
  
  ### Information methods
  
  # Return a Hash of statistics responses from the set of servers. Each value is an array with one entry for each server, in the same order the servers were defined.
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

  # Return a namespaced key for this Memcached instance. Accepts a String <tt>key</tt> value.
  def ns(key) #:doc:
    "#{@namespace}#{key}"
  end
    
  # Checks the return code from Libmemcached against the exception list. Raises the corresponding exception if the return code is not Memcached::Success or Memcached::ActionQueued. Accepts an integer return code.
  def check_return_code(ret) #:doc:
    # XXX 0.14 already returns 0 for an ActionQueued result
    return if ret == 0 or ret == 31
    raise EXCEPTIONS[ret], ""
  end  
    
end
