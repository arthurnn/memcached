
=begin rdoc
The Memcached client class.
=end
class Memcached

  FLAGS = 0x0

  DEFAULTS = {
    :hash => :default,
    :no_block => false,
    :distribution => :consistent_ketama,
    :ketama_weighted => true,
    :buffer_requests => false,
    :cache_lookups => true,
    :support_cas => false,
    :tcp_nodelay => false,
    :show_backtraces => false,
    :retry_timeout => 30,
    :timeout => 0.5,
    :connect_timeout => 2,
    :prefix_key => nil,
    :hash_with_prefix_key => true,
    :default_ttl => 604800,
    :default_weight => 8,
    :sort_hosts => false,
    :auto_eject_hosts => false,
    :server_failure_limit => 2,
    :verify_key => true
  }

#:stopdoc:
  IGNORED = 0
#:startdoc:

  attr_reader :options # Return the options Hash used to configure this instance.

###### Configuration

=begin rdoc
Create a new Memcached instance. Accepts string or array of server strings, as well an an optional configuration hash.

  Memcached.new('localhost', ...) # A single server
  Memcached.new(['web001:11212', 'web002:11212'], ...) # Two servers with custom ports
  Memcached.new(['web001:11211:2', 'web002:11211:8'], ...) # Two servers with default ports and explicit weights

Weights only affect Ketama hashing.  If you use Ketama hashing and don't specify a weight, the client will poll each server's stats and use its size as the weight.

Valid option parameters are:

<tt>:prefix_key</tt>:: A string to prepend to every key, for namespacing. Max length is 127.
<tt>:hash</tt>:: The name of a hash function to use. Possible values are: <tt>:crc</tt>, <tt>:default</tt>, <tt>:fnv1_32</tt>, <tt>:fnv1_64</tt>, <tt>:fnv1a_32</tt>, <tt>:fnv1a_64</tt>, <tt>:hsieh</tt>, <tt>:md5</tt>, and <tt>:murmur</tt>. <tt>:default</tt> is the fastest. Use <tt>:md5</tt> for compatibility with other ketama clients.
<tt>:distribution</tt>:: Either <tt>:modula</tt>, <tt>:consistent_ketama</tt>, <tt>:consistent_wheel</tt>, or <tt>:ketama</tt>. Defaults to <tt>:ketama</tt>.
<tt>:server_failure_limit</tt>:: How many consecutive failures to allow before marking a host as dead. Has no effect unless <tt>:retry_timeout</tt> is also set.
<tt>:retry_timeout</tt>:: How long to wait until retrying a dead server. Has no effect unless <tt>:server_failure_limit</tt> is non-zero. Defaults to <tt>30</tt>.
<tt>:auto_eject_hosts</tt>:: Whether to temporarily eject dead hosts from the pool. Defaults to <tt>false</tt>. Note that in the event of an ejection, <tt>:auto_eject_hosts</tt> will remap the entire pool unless <tt>:distribution</tt> is set to <tt>:consistent</tt>.
<tt>:cache_lookups</tt>:: Whether to cache hostname lookups for the life of the instance. Defaults to <tt>true</tt>.
<tt>:support_cas</tt>:: Flag CAS support in the client. Accepts <tt>true</tt> or <tt>false</tt>. Defaults to <tt>false</tt> because it imposes a slight performance penalty. Note that your server must also support CAS or you will trigger <b>Memcached::ProtocolError</b> exceptions.
<tt>:tcp_nodelay</tt>:: Turns on the no-delay feature for connecting sockets. Accepts <tt>true</tt> or <tt>false</tt>. Performance may or may not change, depending on your system.
<tt>:no_block</tt>:: Whether to use pipelining for writes. Accepts <tt>true</tt> or <tt>false</tt>.
<tt>:buffer_requests</tt>:: Whether to use an internal write buffer. Accepts <tt>true</tt> or <tt>false</tt>. Calling <tt>get</tt> or closing the connection will force the buffer to flush. Note that <tt>:buffer_requests</tt> might not work well without <tt>:no_block</tt> also enabled.
<tt>:show_backtraces</tt>:: Whether <b>Memcached::NotFound</b> exceptions should include backtraces. Generating backtraces is slow, so this is off by default. Turn it on to ease debugging.
<tt>:connect_timeout</tt>:: How long to wait for a connection to a server. Defaults to 2 seconds. Set to <tt>0</tt> if you want to wait forever.
<tt>:timeout</tt>:: How long to wait for a response from the server. Defaults to 0.5 seconds. Set to <tt>0</tt> if you want to wait forever.
<tt>:default_ttl</tt>:: The <tt>ttl</tt> to use on set if no <tt>ttl</tt> is specified, in seconds. Defaults to one week. Set to <tt>0</tt> if you want things to never expire.
<tt>:default_weight</tt>:: The weight to use if <tt>:ketama_weighted</tt> is <tt>true</tt>, but no weight is specified for a server.
<tt>:hash_with_prefix_key</tt>:: Whether to include the prefix when calculating which server a key falls on. Defaults to <tt>true</tt>.
<tt>:sort_hosts</tt>:: Whether to force the server list to stay sorted. This defeats consistent hashing and is rarely useful.
<tt>:verify_key</tt>:: Validate keys before accepting them. Never disable this.

Please note that when pipelining is enabled, setter and deleter methods do not raise on errors. For example, if you try to set an invalid key with <tt>:no_block => true</tt>, it will appear to succeed. The actual setting of the key occurs after libmemcached has returned control to your program, so there is no way to backtrack and raise the exception.

=end

  def initialize(servers, opts = {})
    @struct = Lib::MemcachedSt.new
    Lib.memcached_create(@struct)

    # Merge option defaults and discard meaningless keys
    @options = Hash[*DEFAULTS.map do |key, default|
      [key, opts[key] || default]
    end.flatten]

    # Force :buffer_requests to use :no_block
    # XXX Deleting the :no_block key should also work, but libmemcached doesn't seem to set it
    # consistently
    options[:no_block] = true if options[:buffer_requests]

    # Disallow weights without ketama
    options.delete(:ketama_weighted) if options[:distribution] != :consistent_ketama

    # Legacy accessor
    options[:prefix_key] = options.delete(:namespace) if options[:namespace]

    # Disallow :sort_hosts with consistent hashing
    if options[:sort_hosts] and options[:distribution] == :consistent
      raise ArgumentError, ":sort_hosts defeats :consistent hashing"
    end

    # Set connect timeouts
    if options[:connect_timeout] > 0
      options[:connect_timeout] = (options[:connect_timeout] * 1_000_000).to_i
    end

    # Set send/receive timeouts
    if options[:timeout] > 0
      if options[:no_block]
        options[:poll_timeout] = (options[:timeout] * 1_000).to_i
      else
        options[:poll_timeout] = 1
        options[:rcv_timeout] = (options[:timeout] * 1_000_000).to_i
      end
    elsif options[:no_block]
      raise ArgumentError, "Can't set :timeout => 0 when :no_block => true."
    end

    # Set the behaviors on the struct
    set_behaviors
    set_callbacks

    # Freeze the hash
    options.freeze

    # Set the servers on the struct
    set_servers(servers)

    # Not found exceptions
    unless options[:show_backtraces]
      @not_found_instance = NotFound.new
      @not_found_instance.no_backtrace = true
    end
  end

  # Return the array of server strings used to configure this instance.
  def servers
    server_structs.map do |server|
      inspect_server(server)
    end
  end

  # Safely copy this instance. Returns a Memcached instance.
  #
  # <tt>clone</tt> is useful for threading, since each thread must have its own unshared Memcached
  # object.
  #
  def clone
    memcached = super
    memcached.instance_variable_set('@struct', Lib.memcached_clone(nil, @struct))
    memcached
  end

  # Reset the state of the libmemcached struct. This is useful for changing the server list at runtime.
  def reset(current_servers = nil)
    current_servers ||= servers
    @struct = Lib::MemcachedSt.new
    Lib.memcached_create(@struct)
    set_behaviors
    set_callbacks
    set_servers(current_servers)
  end
  
  # Disconnect from all currently connected servers
  def quit
    Lib.memcached_quit(@struct)
    self
  end

  #:stopdoc:
  alias :dup :clone #:nodoc:
  #:startdoc:

### Configuration helpers

  private

  # Return an array of raw <tt>memcached_host_st</tt> structs for this instance.
  def server_structs
    array = []
    if @struct.hosts
      @struct.hosts.count.times do |i|
        array << Lib.memcached_select_server_at(@struct, i)
      end
    end
    array
  end

###### Operations

  public

### Setters

  # Set a key/value pair. Accepts a String <tt>key</tt> and an arbitrary Ruby object. Overwrites any existing value on the server.
  #
  # Accepts an optional <tt>ttl</tt> value to specify the maximum lifetime of the key on the server. <tt>ttl</tt> can be either an integer number of seconds, or a Time elapsed time object. <tt>0</tt> means no ttl. Note that there is no guarantee that the key will persist as long as the <tt>ttl</tt>, but it will not persist longer.
  #
  # Also accepts a <tt>marshal</tt> value, which defaults to <tt>true</tt>. Set <tt>marshal</tt> to <tt>false</tt> if you want the <tt>value</tt> to be set directly.
  #
  def set(key, value, ttl=nil, marshal=true, flags=FLAGS)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Lib.memcached_set(@struct, key, value, ttl || options[:default_ttl], flags),
      key
    )
  end

  # Add a key/value pair. Raises <b>Memcached::NotStored</b> if the key already exists on the server. The parameters are the same as <tt>set</tt>.
  def add(key, value, ttl=nil, marshal=true, flags=FLAGS)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Lib.memcached_add(@struct, key, value, ttl || options[:default_ttl], flags),
      key
    )
  end

  # Increment a key's value. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist.
  #
  # Also accepts an optional <tt>offset</tt> paramater, which defaults to 1. <tt>offset</tt> must be an integer.
  #
  # Note that the key must be initialized to an unmarshalled integer first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>marshal</tt> set to <tt>false</tt>.
  def increment(key, offset=1)
    ret, value = Lib.memcached_increment(@struct, key, offset)
    check_return_code(ret, key)
    value
  end

  # Decrement a key's value. The parameters and exception behavior are the same as <tt>increment</tt>.
  def decrement(key, offset=1)
    ret, value = Lib.memcached_decrement(@struct, key, offset)
    check_return_code(ret, key)
    value
  end

  #:stopdoc:
  alias :incr :increment
  alias :decr :decrement
  #:startdoc:

  # Replace a key/value pair. Raises <b>Memcached::NotFound</b> if the key does not exist on the server. The parameters are the same as <tt>set</tt>.
  def replace(key, value, ttl=nil, marshal=true, flags=FLAGS)
    value = marshal ? Marshal.dump(value) : value.to_s
    check_return_code(
      Lib.memcached_replace(@struct, key, value, ttl || options[:default_ttl], flags),
      key
    )
  end

  # Appends a string to a key's value. Accepts a String <tt>key</tt> and a String <tt>value</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist on the server.
  #
  # Note that the key must be initialized to an unmarshalled string first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>marshal</tt> set to <tt>false</tt>.
  def append(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Lib.memcached_append(@struct, key, value.to_s, IGNORED, IGNORED),
      key
    )
  end

  # Prepends a string to a key's value. The parameters and exception behavior are the same as <tt>append</tt>.
  def prepend(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Lib.memcached_prepend(@struct, key, value.to_s, IGNORED, IGNORED),
      key
    )
  end

  # Reads a key's value from the server and yields it to a block. Replaces the key's value with the result of the block as long as the key hasn't been updated in the meantime, otherwise raises <b>Memcached::NotStored</b>. Accepts a String <tt>key</tt> and a block.
  #
  # Also accepts an optional <tt>ttl</tt> value.
  #
  # CAS stands for "compare and swap", and avoids the need for manual key mutexing. CAS support must be enabled in Memcached.new or a <b>Memcached::ClientError</b> will be raised. Note that CAS may be buggy in memcached itself.
  #
  def cas(key, ttl=nil, marshal=true, flags=FLAGS)
    raise ClientError, "CAS not enabled for this Memcached instance" unless options[:support_cas]

    value = get(key, marshal)
    value = yield value
    value = marshal ? Marshal.dump(value) : value.to_s

    check_return_code(
      Lib.memcached_cas(@struct, key, value, ttl || options[:default_ttl], flags, @struct.result.cas),
      key
    )
  end

### Deleters

  # Deletes a key/value pair from the server. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist.
  def delete(key)
    check_return_code(
      Lib.memcached_delete(@struct, key, IGNORED),
      key
    )
  end

  # Flushes all key/value pairs from all the servers.
  def flush
    check_return_code(
      Lib.memcached_flush(@struct, IGNORED)
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
      ret = Lib.memcached_mget(@struct, keys);
      check_return_code(ret, keys)

      hash = {}
      keys.size.times do
        value, key, flags, ret = Lib.memcached_fetch_rvalue(@struct)
        break if ret == Lib::MEMCACHED_END
        check_return_code(ret, key)
        value = Marshal.load(value) if marshal
        # Assign the value
        hash[key] = value
      end
      hash
    else
      # Single get
      value, flags, ret = Lib.memcached_get_rvalue(@struct, keys)
      check_return_code(ret, keys)
      value = Marshal.load(value) if marshal
      value
    end
  end

  ### Information methods

  # Return the server used by a particular key.
  def server_by_key(key)
    ret = Lib.memcached_server_by_key(@struct, key)    
    inspect_server(ret.first) if ret.is_a?(Array)
  end

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
         check_return_code(ret, key)

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

  # Checks the return code from Rlibmemcached against the exception list. Raises the corresponding exception if the return code is not Memcached::Success or Memcached::ActionQueued. Accepts an integer return code and an optional key, for exception messages.
  def check_return_code(ret, key = nil) #:doc:
    return if ret == 0 # Lib::MEMCACHED_SUCCESS
    return if ret == Lib::MEMCACHED_BUFFERED

    if ret == Lib::MEMCACHED_NOTFOUND and !options[:show_backtraces]
      raise @not_found_instance
    else
      message = "Key #{inspect_keys(key).inspect}"
      if ret == Lib::MEMCACHED_ERRNO and key.is_a?(String)
        server = Lib.memcached_server_by_key(@struct, key)
        message += ", errno #{server.first.cached_errno}."
      end
      raise EXCEPTIONS[ret], message
    end
  end

  # Turn an array of keys into a hash of keys to servers.
  def inspect_keys(keys)
    Hash[*Array(keys).map do |key|
      [key, server_by_key(key)]
    end.flatten]
  end

  # Set the servers on the struct.
  def set_servers(servers)
    Array(servers).each_with_index do |server, index|
      unless server.is_a? String and server =~ /^[\w\d\.-]+(:\d{1,5}){0,2}$/
        raise ArgumentError, "Servers must be in the format host:port[:weight] (e.g., 'localhost:11211' or  'localhost:11211:10')"
      end
      host, port, weight = server.split(":")
      Lib.memcached_server_add_with_weight(@struct, host, port.to_i, (weight || options[:default_weight]).to_i)
    end
    # For inspect
    @servers = send(:servers)
  end

  # Set the behaviors on the struct from the current options.
  def set_behaviors
    BEHAVIORS.keys.each do |behavior|
      set_behavior(behavior, options[behavior]) if options.key?(behavior)
    end
  end

  # Set the callbacks on the struct from the current options.
  def set_callbacks
    # Only support prefix_key for now
    if options[:prefix_key]
      unless options[:prefix_key].size < Lib::MEMCACHED_PREFIX_KEY_MAX_SIZE
        raise ArgumentError, "Max prefix_key size is #{Lib::MEMCACHED_PREFIX_KEY_MAX_SIZE - 1}"
      end
      Lib.memcached_callback_set(@struct, Lib::MEMCACHED_CALLBACK_PREFIX_KEY, options[:prefix_key])
    end
  end

  # Stringify an opaque server struct.
  def inspect_server(server)
    "#{server.hostname}:#{server.port}#{":#{server.weight}" if options[:ketama_weighted]}"
  end

end
