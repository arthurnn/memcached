
=begin rdoc
The Memcached client class.
=end
class Memcached

  FLAGS = 0x0

  DEFAULTS = {
    :hash => :default,
    :no_block => false,
    :distribution => :consistent,
    :buffer_requests => false,
    :cache_lookups => true,
    :support_cas => false,
    :tcp_nodelay => false,
    :show_not_found_backtraces => false,
    :retry_timeout => 60,
#    :poll_timeout => 5,
    :connect_timeout => 5,
    :namespace => nil,
    :sort_hosts => false,
    :failover => true
  } 
      
  # :verify_key => false # XXX We do this ourselves already in Rlibmemcached.ns()
  
#:stopdoc:
  IGNORED = 0
  
  NOTFOUND_INSTANCE = NotFound.new
#:startdoc:
  
  attr_reader :options # Return the options Hash used to configure this instance.

###### Configuration

=begin rdoc      
Create a new Memcached instance. Accepts a single server string such as '127.0.0.1:11211', or an array of such strings, as well an an optional configuration hash.

Hostname lookups are not currently supported; you need to use the IP address.

Valid option parameters are:

<tt>:namespace</tt>:: A namespace string to prepend to every key.
<tt>:hash</tt>:: The name of a hash function to use. Possible values are: <tt>:crc</tt>, <tt>:default</tt>, <tt>:fnv1_32</tt>, <tt>:fnv1_64</tt>, <tt>:fnv1a_32</tt>, <tt>:fnv1a_64</tt>, <tt>:hsieh</tt>, <tt>:md5</tt>, and <tt>:murmur</tt>. <tt>:default</tt> is the fastest. Use <tt>:md5</tt> for compatibility with other ketama clients.
<tt>:distribution</tt>:: Either <tt>:modula</tt>, <tt>:consistent</tt>, or <tt>:consistent_wheel</tt>. Default <tt>:consistent</tt>.
<tt>:failover</tt>:: Whether to permanently eject failed hosts from the pool. Defaults to <tt>true</tt>.
<tt>:support_cas</tt>:: Flag CAS support in the client. Accepts <tt>true</tt> or <tt>false</tt>. Note that your server must also support CAS or you will trigger <b>Memcached::ProtocolError</b> exceptions.
<tt>:tcp_nodelay</tt>:: Turns on the no-delay feature for connecting sockets. Accepts <tt>true</tt> or <tt>false</tt>. Performance may or may not change, depending on your system.
<tt>:no_block</tt>:: Whether to use non-blocking, asynchronous IO for writes. Accepts <tt>true</tt> or <tt>false</tt>.
<tt>:buffer_requests</tt>:: Whether to use an internal write buffer. Accepts <tt>true</tt> or <tt>false</tt>. Calling <tt>get</tt> or closing the connection will force the buffer to flush. Note that <tt>:buffer_requests</tt> might not work well without <tt>:no_block</tt> also enabled.
<tt>:show_not_found_backtraces</tt>:: Whether <b>Memcached::NotFound</b> exceptions should include backtraces. Generating backtraces is slow, so this is off by default. Turn it on to ease debugging.
<tt>:sort_hosts</tt>:: Whether to force the server list to stay sorted. This defeats consistent hashing.

Please note that when non-blocking IO is enabled, setter and deleter methods do not raise on errors. For example, if you try to set an invalid key with <tt>:no_block => true</tt>, it will appear to succeed. The actual setting of the key occurs after libmemcached has returned control to your program, so there is no way to backtrack and raise the exception.

=end

  def initialize(servers, opts = {})
    @struct = Lib::MemcachedSt.new    
    Lib.memcached_create(@struct)

    # Set the servers on the struct
    set_servers(servers)
    
    # Merge option defaults
    @options = DEFAULTS.merge(opts)
    
    # Force :buffer_requests to use :no_block
    # XXX Deleting the :no_block key should also work, but libmemcached doesn't seem to set it
    # consistently
    options[:no_block] = true if options[:buffer_requests] 
    
    # Disallow :sort_hosts with consistent hashing
    if options[:sort_hosts] and options[:distribution] == :consistent
      raise ArgumentError, ":sort_hosts defeats :consistent hashing"
    end
    
    # Set the behaviors on the struct
    set_behaviors
     
    # Merge the actual behaviors back in
    BEHAVIORS.keys.each do |behavior|
      options[behavior] = get_behavior(behavior)
    end    

    # Freeze the hash
    options.freeze
        
    # Namespace
    raise ArgumentError, "Invalid namespace" if options[:namespace].to_s =~ / /
    @namespace = options[:namespace].to_s
    @namespace_size = @namespace.size
    
    # Not found exceptions
    # Note that these have global effects since the NotFound class itself is modified. You should only 
    # be enabling the backtrace for debugging purposes, so it's not really a big deal.
    if options[:show_not_found_backtraces]
      NotFound.restore_backtraces
    else
      NotFound.remove_backtraces
    end
    
  end

  # Return the array of server strings used to configure this instance.
  def servers
    server_structs.map do |server|
      "#{server.hostname}:#{server.port}"
    end
  end

  # Safely copy this instance. Returns a Memcached instance. 
  #
  # <tt>clone</tt> is useful for threading, since each thread must have its own unshared Memcached 
  # object. However, you must call destroy before each thread returns or you will leak significant memory.
  #
  def clone
    memcached = super
    memcached.instance_variable_set('@struct', Lib.memcached_clone(nil, @struct))
    memcached
  end
  
  # Destroy this instance. Frees memory associated with the C implementation.
  #
  # Accepts an optional parameter <tt>disable_methods</tt>. When <tt>false</tt>, destroy
  # runs much faster, but your instance will segfault if you try to call any other methods on it
  # after destroy. Defaults to <tt>true</tt>, which safely overwrites all instance methods.
  def destroy(disable_methods = true)
    Lib.memcached_free(@struct)
    @struct = nil
    
    if disable_methods
      class << self
        Memcached.instance_methods.each do |method_name|
          define_method method_name do |*args|
            raise Memcached::ClientError, "Instance has been explicitly destroyed"
          end
        end
      end
    end
  end  
  
  # Reset the state of the libmemcached struct. Fixes out-of-sync errors with the Memcached pool.
  def reset(current_servers = nil)
    current_servers ||= servers
    Lib.memcached_free(@struct)        
    @struct = Lib::MemcachedSt.new    
    Lib.memcached_create(@struct)
    set_servers(current_servers)
    set_behaviors
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
      array << Lib.memcached_select_server_at(@struct, i)
    end
    array
  end    
    
###### Operations
  
  public
  
### Setters
    
  # Set a key/value pair. Accepts a String <tt>key</tt> and an arbitrary Ruby object. Overwrites any existing value on the server.
  #
  # Accepts an optional <tt>timeout</tt> value to specify the maximum lifetime of the key on the server. <tt>timeout</tt> can be either an integer number of seconds, or a Time elapsed time object. <tt>0</tt> means no timeout. Note that there is no guarantee that the key will persist as long as the <tt>timeout</tt>, but it will not persist longer.
  #
  # Also accepts a <tt>marshal</tt> value, which defaults to <tt>true</tt>. Set <tt>marshal</tt> to <tt>false</tt> if you want the <tt>value</tt> to be set directly. 
  # 
  def set(key, value, timeout=0, marshal=true)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Lib.memcached_set(@struct, Lib.ns(@namespace, key), value, timeout, FLAGS)
    )
  end

  # Add a key/value pair. Raises <b>Memcached::NotStored</b> if the key already exists on the server. The parameters are the same as <tt>set</tt>.
  def add(key, value, timeout=0, marshal=true)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Lib.memcached_add(@struct, Lib.ns(@namespace, key), value, timeout, FLAGS)
    )
  end

  # Increment a key's value. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist. 
  #
  # Also accepts an optional <tt>offset</tt> paramater, which defaults to 1. <tt>offset</tt> must be an integer.
  #
  # Note that the key must be initialized to an unmarshalled integer first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>marshal</tt> set to <tt>false</tt>.
  def increment(key, offset=1)
    ret, value = Lib.memcached_increment(@struct, Lib.ns(@namespace, key), offset)
    check_return_code(ret)
    value
  end

  # Decrement a key's value. The parameters and exception behavior are the same as <tt>increment</tt>.
  def decrement(key, offset=1)
    ret, value = Lib.memcached_decrement(@struct, Lib.ns(@namespace, key), offset)
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
      Lib.memcached_replace(@struct, Lib.ns(@namespace, key), value, timeout, FLAGS)
    )
  end

  # Appends a string to a key's value. Accepts a String <tt>key</tt> and a String <tt>value</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist on the server. 
  #
  # Note that the key must be initialized to an unmarshalled string first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>marshal</tt> set to <tt>false</tt>.
  def append(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Lib.memcached_append(@struct, Lib.ns(@namespace, key), value.to_s, IGNORED, FLAGS)
    )
  end
  
  # Prepends a string to a key's value. The parameters and exception behavior are the same as <tt>append</tt>.
  def prepend(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Lib.memcached_prepend(@struct, Lib.ns(@namespace, key), value.to_s, IGNORED, FLAGS)
    )
  end
  
  # Reads a key's value from the server and yields it to a block. Replaces the key's value with the result of the block as long as the key hasn't been updated in the meantime, otherwise raises <b>Memcached::NotStored</b>. Accepts a String <tt>key</tt> and a block.
  #
  # Also accepts an optional <tt>timeout</tt> value.
  #
  # CAS stands for "compare and swap", and avoids the need for manual key mutexing. CAS support must be enabled in Memcached.new or a <b>Memcached::ClientError</b> will be raised. Note that CAS may be buggy in memcached itself.
  #
  def cas(key, timeout = 0, marshal = true)
    raise ClientError, "CAS not enabled for this Memcached instance" unless options[:support_cas]
      
    value = get(key, marshal)
    value = yield value
    value = marshal ? Marshal.dump(value) : value.to_s
    
    check_return_code(
      Lib.memcached_cas(@struct, Lib.ns(@namespace, key), value, timeout, FLAGS, @struct.result.cas)
    )
  end

### Deleters

  # Deletes a key/value pair from the server. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist.
  def delete(key)
    check_return_code(
      Lib.memcached_delete(@struct, Lib.ns(@namespace, key), IGNORED)
    )  
  end
  
  # Flushes all key/value pairs from the server.
  def flush
    check_return_code(
      Lib.memcached_flush(@struct, 0)
    )
  end
  
### Getters  
  
  # Gets a key's value from the server. Accepts a String <tt>key</tt> or array of String <tt>keys</tt>.
  #
  # Also accepts a <tt>marshal</tt> value, which defaults to <tt>true</tt>. Set <tt>marshal</tt> to <tt>false</tt> if you want the <tt>value</tt> to be returned directly as a String. Otherwise it will be assumed to be a marshalled Ruby object and unmarshalled.
  #
  # If you pass a String key, and the key does not exist on the server, <b>Memcached::NotFound</b> will be raised. If you pass an array of keys, memcached's <tt>multiget</tt> mode will be used, and a hash of key/value pairs will be returned. The hash will contain only the keys that were found.
  #
  # The multiget behavior is subject to change in the future; however, for multiple lookups, it is much faster than normal mode.
  #
  # Note that when you rescue Memcached::NotFound exceptions, you should use a the block rescue syntax instead of the inline syntax. Block rescues are very fast, but inline rescues are very slow.
  #
  def get(keys, marshal=true)
    if keys.is_a? Array
      # Multi get
      keys.map! { |key| Lib.ns(@namespace, key) }
      hash = {}
      
      Lib.memcached_mget(@struct, keys);
      
      keys.size.times do 
        value, key, flags, ret = Lib.memcached_fetch_rvalue(@struct)
        break if ret == Lib::MEMCACHED_END
        check_return_code(ret)
        value = Marshal.load(value) if marshal
        # Assign the value, removing the namespace, if present
        hash[key[@namespace_size..-1]] = value
      end
      hash
    else
      # Single get
      value, flags, ret = Lib.memcached_get_rvalue(@struct, Lib.ns(@namespace, keys))
      check_return_code(ret)
      value = Marshal.load(value) if marshal
      value
    end    
  end    
  
  ### Information methods
  
  # Return a Hash of statistics responses from the set of servers. Each value is an array with one entry for each server, in the same order the servers were defined.
  def stats
    stats = Hash.new([])
    
    stat_struct, ret = Lib.memcached_stat(@struct, "")
    check_return_code(ret)
    
    keys, ret = Lib.memcached_stat_get_keys(@struct, stat_struct)
    check_return_code(ret)
    
    keys.each do |key|
       server_structs.size.times do |index|

         value, ret = Lib.memcached_stat_get_rvalue(
           @struct, 
           Lib.memcached_select_stat_at(@struct, stat_struct, index),
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
    
    Lib.memcached_stat_free(@struct, stat_struct)
    stats
  end  
  
  ### Operations helpers
  
  private
  
  # Returns the hash value for a master key
  def hash(key)
    Lib.memcached_generate_hash(@struct, Lib.ns(@namespace, key))
  end
    
  # Checks the return code from Rlibmemcached against the exception list. Raises the corresponding exception if the return code is not Memcached::Success or Memcached::ActionQueued. Accepts an integer return code.
  def check_return_code(ret) #:doc:
    # 0.16 --enable-debug returns 0 for an ActionQueued result but --disable-debug does not
    return if ret == 0 or ret == 31

    # SystemError; eject from the pool
    if ret == 25 and options[:failover]
      failed = sweep_servers
      raise EXCEPTIONS[ret], "Server #{failed} failed permanently"
    else
      raise EXCEPTIONS[ret], ""      
    end
  end  

  # Eject the first dead server we find from the pool and reset the struct
  def sweep_servers
    server_structs.each do |server|
      if server.next_retry > Time.now 
        server_name = "#{server.hostname}:#{server.port}"
        current_servers = servers
        current_servers.delete(server_name) 
        reset(current_servers)
        return server_name        
      end
    end
    "(unknown)"
  end
  
  # Set the servers on the struct
  def set_servers(servers)
    Array(servers).each_with_index do |server, index|
      unless server.is_a? String and server =~ /^(\d{1,3}\.){3}\d{1,3}:\d{1,5}$/
        raise ArgumentError, "Servers must be in the format ip:port (e.g., '127.0.0.1:11211')" 
      end
      host, port = server.split(":")
      Lib.memcached_server_add(@struct, host, port.to_i)
    end    
  end
  
  # Set the behaviors on the struct from the current options
  def set_behaviors
    options.each do |option, value|
      unless [:namespace, :show_not_found_backtraces, :failover].include? option
        set_behavior(option, value) 
      end
    end   
  end  
    
end
