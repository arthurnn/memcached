=begin rdoc
The Memcached client class.
=end
class Memcached
  FLAGS = 0x0

  DEFAULTS = {
    :hash => :fnv1_32,
    :no_block => false,
    :noreply => false,
    :distribution => :consistent_ketama,
    :ketama_weighted => true,
    :buffer_requests => false,
    :cache_lookups => true,
    :support_cas => false,
    :tcp_nodelay => false,
    :show_backtraces => false,
    :retry_timeout => 60,
    :timeout => 0.25,
    :rcv_timeout => nil,
    :snd_timeout => nil,
    :poll_max_retries => 1,
    :poll_timeout => nil,
    :connect_timeout => 0.25,
    :prefix_key => '',
    :prefix_delimiter => '',
    :hash_with_prefix_key => true,
    :default_ttl => 604800,
    :default_weight => 8,
    :sort_hosts => false,
    :auto_eject_hosts => true,
    :server_failure_limit => 2,
    :verify_key => true,
    :use_udp => false,
    :binary_protocol => false,
    :credentials => nil,
    :experimental_features => false,
    :codec => Memcached::MarshalCodec,
    :exception_retry_limit => 5,
    :exceptions_to_retry => [
        Memcached::ServerIsMarkedDead,
        Memcached::ATimeoutOccurred,
        Memcached::ConnectionBindFailure,
        Memcached::ConnectionFailure,
        Memcached::ConnectionSocketCreateFailure,
        Memcached::Failure,
        Memcached::MemoryAllocationFailure,
        Memcached::ReadFailure,
        Memcached::ServerEnd,
        Memcached::ServerError,
        Memcached::SystemError,
        Memcached::UnknownReadFailure,
        Memcached::WriteFailure,
        Memcached::SomeErrorsWereReported]
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

<tt>:prefix_key</tt>:: A string to prepend to every key, for namespacing. Max length is 127. Defaults to the empty string.
<tt>:prefix_delimiter</tt>:: A character to append to the prefix key. Defaults to the empty string.
<tt>:codec</tt>:: An object to use as the codec for encoded requests. Defaults to Memcached::MarshalCodec.
<tt>:hash</tt>:: The name of a hash function to use. Possible values are: <tt>:crc</tt>, <tt>:default</tt>, <tt>:fnv1_32</tt>, <tt>:fnv1_64</tt>, <tt>:fnv1a_32</tt>, <tt>:fnv1a_64</tt>, <tt>:hsieh</tt>, <tt>:md5</tt>, <tt>:murmur</tt>, and <tt>:none</tt>. <tt>:fnv1_32</tt> is fast and well known, and is the default. Use <tt>:md5</tt> for compatibility with other ketama clients. <tt>:none</tt> is for use when there is a single server, and performs no actual hashing.
<tt>:distribution</tt>:: Either <tt>:modula</tt>, <tt>:consistent_ketama</tt>, <tt>:consistent_wheel</tt>, or <tt>:ketama</tt>. Defaults to <tt>:ketama</tt>.
<tt>:server_failure_limit</tt>:: How many consecutive failures to allow before marking a host as dead. Has no effect unless <tt>:retry_timeout</tt> is also set.
<tt>:retry_timeout</tt>:: How long to wait until retrying a dead server. Has no effect unless <tt>:server_failure_limit</tt> is non-zero. Defaults to <tt>30</tt>.
<tt>:auto_eject_hosts</tt>:: Whether to temporarily eject dead hosts from the pool. Defaults to <tt>true</tt>. Note that in the event of an ejection, <tt>:auto_eject_hosts</tt> will remap the entire pool unless <tt>:distribution</tt> is set to <tt>:consistent</tt>.
<tt>:exception_retry_limit</tt>:: How many times to retry before raising exceptions in <tt>:exceptions_to_retry</tt>. Defaults to <tt>5</tt>.
<tt>:exceptions_to_retry</tt>:: Which exceptions to retry. Defaults to <b>ServerIsMarkedDead</b>, <b>ATimeoutOccurred</b>, <b>ConnectionBindFailure</b>, <b>ConnectionFailure</b>, <b>ConnectionSocketCreateFailure</b>, <b>Failure</b>, <b>MemoryAllocationFailure</b>, <b>ReadFailure</b>, <b>ServerError</b>, <b>SystemError</b>, <b>UnknownReadFailure</b>, and <b>WriteFailure</b>.
<tt>:cache_lookups</tt>:: Whether to cache hostname lookups for the life of the instance. Defaults to <tt>true</tt>.
<tt>:support_cas</tt>:: Flag CAS support in the client. Accepts <tt>true</tt> or <tt>false</tt>. Defaults to <tt>false</tt> because it imposes a slight performance penalty. Note that your server must also support CAS or you will trigger <b>ProtocolError</b> exceptions.
<tt>:tcp_nodelay</tt>:: Turns on the no-delay feature for connecting sockets. Accepts <tt>true</tt> or <tt>false</tt>. Performance may or may not change, depending on your system.
<tt>:no_block</tt>:: Whether to use pipelining for writes. Defaults to <tt>false</tt>.
<tt>:buffer_requests</tt>:: Whether to use an internal write buffer. Accepts <tt>true</tt> or <tt>false</tt>. Calling <tt>get</tt> or closing the connection will force the buffer to flush. Client behavior is undefined unless <tt>:no_block</tt> is enabled. Defaults to <tt>false</tt>.
<tt>:noreply</tt>:: Ask server not to reply for storage commands. Client behavior is undefined unless <tt>:no_block</tt> and <tt>:buffer_requests</tt> are enabled. Defaults to <tt>false</tt>.
<tt>:show_backtraces</tt>:: Whether <b>NotFound</b> and <b>NotStored</b> exceptions should include backtraces. Generating backtraces is slow, so this is off by default. Turn it on to ease debugging.
<tt>:connect_timeout</tt>:: How long to wait for a connection to a server. Defaults to 2 seconds. Set to <tt>0</tt> if you want to wait forever.
<tt>:timeout</tt>:: How long to wait for a response from the server. Defaults to 0.25 seconds. Set to <tt>0</tt> if you want to wait forever.
<tt>:default_ttl</tt>:: The <tt>ttl</tt> to use on set if no <tt>ttl</tt> is specified, in seconds. Defaults to one week. Set to <tt>0</tt> if you want things to never expire.
<tt>:default_weight</tt>:: The weight to use if <tt>:ketama_weighted</tt> is <tt>true</tt>, but no weight is specified for a server.
<tt>:hash_with_prefix_key</tt>:: Whether to include the prefix when calculating which server a key falls on. Defaults to <tt>true</tt>.
<tt>:use_udp</tt>:: Use the UDP protocol to reduce connection overhead. Defaults to false.
<tt>:binary_protocol</tt>:: Use the binary protocol. Defaults to false. Please note that using the binary protocol is usually <b>slower</b> than the ASCII protocol.
<tt>:sort_hosts</tt>:: Whether to force the server list to stay sorted. This defeats consistent hashing and is rarely useful.
<tt>:verify_key</tt>:: Validate keys before accepting them. Never disable this.
<tt>:poll_max_retries</tt>:: Maximum poll timeout retries before marking a flush failed on timeouts.

Please note that when <tt>:no_block => true</tt>, update methods do not raise on errors. For example, if you try to <tt>set</tt> an invalid key, it will appear to succeed. The actual setting of the key occurs after libmemcached has returned control to your program, so there is no way to backtrack and raise the exception.

=end

  def initialize(servers = nil, opts = {})
    @struct = Lib.memcached_create(nil)

    # Merge option defaults and discard meaningless keys
    @options = DEFAULTS.merge(opts)
    @options.each do |key,_|
       unless DEFAULTS.keys.include? key
         raise ArgumentError, "#{key.inspect} is not a valid configuration parameter."
       end
    end

    # Marginally speed up settings access for hot paths
    @default_ttl = options[:default_ttl]
    @codec = options[:codec]
    @support_cas = options[:support_cas]

    if servers == nil || servers == []
      if ENV.key?("MEMCACHE_SERVERS")
        servers = ENV["MEMCACHE_SERVERS"].split(",").map do | s | s.strip end
      else
        servers = "127.0.0.1:11211"
      end
    end

    if !options[:credentials] and ENV["MEMCACHE_USERNAME"] and ENV["MEMCACHE_PASSWORD"]
      options[:credentials] = [ENV["MEMCACHE_USERNAME"], ENV["MEMCACHE_PASSWORD"]]
    end

    instance_eval { send(:extend, Experimental) } if options[:experimental_features]

    # SASL requires binary protocol
    options[:binary_protocol] = true if options[:credentials]

    # UDP requires noreply
    options[:noreply] = true if options[:use_udp]

    # Buffering requires non-blocking
    # FIXME This should all be wrapped up in a single :pipeline option.
    options[:no_block] = true if options[:buffer_requests]

    # Disallow weights without ketama
    options.delete(:ketama_weighted) if options[:distribution] != :consistent_ketama

    # Disallow :sort_hosts with consistent hashing
    if options[:sort_hosts] and options[:distribution] == :consistent
      raise ArgumentError, ":sort_hosts defeats :consistent hashing"
    end

    # Read timeouts
    options[:rcv_timeout] ||= options[:timeout]
    options[:poll_timeout] ||= options[:timeout]

    # Write timeouts
    options[:snd_timeout] ||= options[:timeout]

    # Set the prefix key
    set_prefix_key(options[:prefix_key])

    # Set the behaviors and credentials on the struct
    set_behaviors
    set_credentials

    # Freeze the hash
    options.freeze

    # Set the servers on the struct
    set_servers(servers)

    # Not found exceptions
    if options[:show_backtraces]
      # Raise classes for full context
      @not_found = NotFound
      @not_stored = NotStored
    else
      # Raise fast context-free exception instances
      @not_found = NotFound.new
      @not_found.no_backtrace = true
      @not_stored = NotStored.new
      @not_stored.no_backtrace = true
    end
  end

  # Set the server list.
  # FIXME Does not necessarily free any existing server structs.
  def set_servers(servers)
    Array(servers).each_with_index do |server, index|
      # Socket
      check_return_code(
        if server.is_a?(String) and File.socket?(server)
          args = [@struct, server, options[:default_weight].to_i]
          Lib.memcached_server_add_unix_socket_with_weight(*args)
        # Network
        elsif server.is_a?(String) and server =~ /^[\w\d\.-]+(:\d{1,5}){0,2}$/
          host, port, weight = server.split(":")
          args = [@struct, host, port.to_i, (weight || options[:default_weight]).to_i]
          if options[:use_udp]
            Lib.memcached_server_add_udp_with_weight(*args)
          else
            Lib.memcached_server_add_with_weight(*args)
          end
        else
          raise ArgumentError, "Servers must be either in the format 'host:port[:weight]' (e.g., 'localhost:11211' or  'localhost:11211:10') for a network server, or a valid path to a Unix domain socket (e.g., /var/run/memcached)."
        end
      )
    end
  end

  # Return the array of server strings used to configure this instance.
  def servers
    server_structs.map do |server|
      inspect_server(server)
    end
  end

  # Set the prefix key.
  def set_prefix_key(key)
    check_return_code(
      if key
        key += options[:prefix_delimiter]
        raise ArgumentError, "Max prefix key + prefix delimiter size is #{Lib::MEMCACHED_PREFIX_KEY_MAX_SIZE - 1}" unless
          key.size < Lib::MEMCACHED_PREFIX_KEY_MAX_SIZE
        Lib.memcached_callback_set(@struct, Lib::MEMCACHED_CALLBACK_PREFIX_KEY, key)
      else
        Lib.memcached_callback_set(@struct, Lib::MEMCACHED_CALLBACK_PREFIX_KEY, "")
      end
    )
  end

  # Return the current prefix key.
  def prefix_key
    if @struct.prefix_key.size > 0
      @struct.prefix_key[0..-1 - options[:prefix_delimiter].size]
    else
      ""
    end
  end

  # Safely copy this instance. Returns a Memcached instance.
  #
  # <tt>clone</tt> is useful for threading, since each thread must have its own unshared Memcached
  # object.
  #
  def clone
    memcached = super
    struct = Lib.memcached_clone(nil, @struct)
    memcached.instance_variable_set('@struct', struct)
    memcached
  end

  # Reset the state of the libmemcached struct. This is useful for changing the server list at runtime.
  def reset(current_servers = nil)
    # Store state and teardown
    current_servers ||= servers
    prev_prefix_key = prefix_key
    quit

    # Create
    # FIXME Duplicates logic with initialize()
    @struct = Lib.memcached_create(nil)
    set_prefix_key(prev_prefix_key)
    set_behaviors
    set_credentials
    set_servers(current_servers)
  end

  # Disconnect from all currently connected servers
  def quit
    Lib.memcached_quit(@struct)
    self
  end

  # Should retry the exception
  def should_retry(e)
    options[:exceptions_to_retry].each {|ex_class| return true if e.instance_of?(ex_class)}
    false
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
  # Accepts an optional <tt>ttl</tt> value to specify the maximum lifetime of the key on the server, in seconds. <tt>ttl</tt> must be a <tt>FixNum</tt>. <tt>0</tt> means no ttl. Note that there is no guarantee that the key will persist as long as the <tt>ttl</tt>, but it will not persist longer.
  #
  # Also accepts an <tt>encode</tt> value, which defaults to <tt>true</tt> and uses the default Marshal codec. Set <tt>encode</tt> to <tt>false</tt>, and pass a String as the <tt>value</tt>, if you want to set a raw byte array.
  #
  def set(key, value, ttl=@default_ttl, encode=true, flags=FLAGS)
    value, flags = @codec.encode(key, value, flags) if encode
    begin
      check_return_code(
        Lib.memcached_set(@struct, key, value, ttl, flags),
        key
      )
    rescue => e
      tries ||= 0
      retry if e.instance_of?(ClientError) && !tries
      raise unless tries < options[:exception_retry_limit] && should_retry(e)
      tries += 1
      retry
    end
  end

  # Add a key/value pair. Raises <b>Memcached::NotStored</b> if the key already exists on the server. The parameters are the same as <tt>set</tt>.
  def add(key, value, ttl=@default_ttl, encode=true, flags=FLAGS)
    value, flags = @codec.encode(key, value, flags) if encode
    begin
      check_return_code(
        Lib.memcached_add(@struct, key, value, ttl, flags),
        key
      )
    rescue => e
      tries ||= 0
      raise unless tries < options[:exception_retry_limit] && should_retry(e)
      tries += 1
      retry
    end
  end

  # Increment a key's value. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist.
  #
  # Also accepts an optional <tt>offset</tt> paramater, which defaults to 1. <tt>offset</tt> must be an integer.
  #
  # Note that the key must be initialized to an unencoded integer first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>encode</tt> set to <tt>false</tt>.
  def increment(key, offset=1)
    ret, value = Lib.memcached_increment(@struct, key, offset)
    check_return_code(ret, key)
    value
  rescue => e
    tries ||= 0
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

  # Decrement a key's value. The parameters and exception behavior are the same as <tt>increment</tt>.
  def decrement(key, offset=1)
    ret, value = Lib.memcached_decrement(@struct, key, offset)
    check_return_code(ret, key)
    value
  rescue => e
    tries ||= 0
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

  #:stopdoc:
  alias :incr :increment
  alias :decr :decrement
  #:startdoc:

  # Replace a key/value pair. Raises <b>Memcached::NotFound</b> if the key does not exist on the server. The parameters are the same as <tt>set</tt>.
  def replace(key, value, ttl=@default_ttl, encode=true, flags=FLAGS)
    value, flags = @codec.encode(key, value, flags) if encode
    begin
      check_return_code(
        Lib.memcached_replace(@struct, key, value, ttl, flags),
        key
      )
    rescue => e
      tries ||= 0
      raise unless tries < options[:exception_retry_limit] && should_retry(e)
      tries += 1
      retry
    end
  end

  # Appends a string to a key's value. Accepts a String <tt>key</tt> and a String <tt>value</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist on the server.
  #
  # Note that the key must be initialized to an unencoded string first, via <tt>set</tt>, <tt>add</tt>, or <tt>replace</tt> with <tt>encode</tt> set to <tt>false</tt>.
  def append(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Lib.memcached_append(@struct, key, value.to_s, IGNORED, IGNORED),
      key
    )
  rescue => e
    tries ||= 0
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

  # Prepends a string to a key's value. The parameters and exception behavior are the same as <tt>append</tt>.
  def prepend(key, value)
    # Requires memcached 1.2.4
    check_return_code(
      Lib.memcached_prepend(@struct, key, value.to_s, IGNORED, IGNORED),
      key
    )
  rescue => e
    tries ||= 0
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

  # Reads a key's value from the server and yields it to a block. Replaces the key's value with the result of the block as long as the key hasn't been updated in the meantime, otherwise raises <b>Memcached::NotStored</b>. Accepts a String <tt>key</tt> and a block.
  #
  # Also accepts an optional <tt>ttl</tt> value.
  #
  # CAS stands for "compare and swap", and avoids the need for manual key mutexing. CAS support must be enabled in Memcached.new or a <b>Memcached::ClientError</b> will be raised. Note that CAS may be buggy in memcached itself.
  # :retry_on_exceptions does not apply to this method
  def cas(keys, ttl=@default_ttl, decode=true)
    raise ClientError, "CAS not enabled for this Memcached instance" unless options[:support_cas]
    tries ||= 0

    if keys.is_a? Array
      # Multi CAS
      hash, flags_and_cas = multi_get(keys, decode)
      unless hash.empty?
        hash = yield hash
        # Only CAS entries that were updated from the original hash
        hash.delete_if {|k, _| !flags_and_cas.has_key?(k) }
        hash = multi_cas(hash, ttl, flags_and_cas, decode, tries)
      end
      hash
    else
      # Single CAS
      value, flags, cas = single_get(keys, decode)
      value = yield value
      single_cas(keys, value, ttl, flags, cas, decode)
    end
  rescue => e
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

  alias :compare_and_swap :cas

### Deleters

  # Deletes a key/value pair from the server. Accepts a String <tt>key</tt>. Raises <b>Memcached::NotFound</b> if the key does not exist.
  def delete(key)
    check_return_code(
      Lib.memcached_delete(@struct, key, IGNORED),
      key
    )
  rescue => e
    tries ||= 0
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

  # Flushes all key/value pairs from all the servers.
  def flush
    check_return_code(
      Lib.memcached_flush(@struct, IGNORED)
    )
  rescue => e
    tries ||= 0
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

### Getters

  # Gets a key's value from the server. Accepts a String <tt>key</tt> or array of String <tt>keys</tt>.
  #
  # Also accepts a <tt>decode</tt> value, which defaults to <tt>true</tt>. Set <tt>decode</tt> to <tt>false</tt> if you want the <tt>value</tt> to be returned directly as a String. Otherwise it will be assumed to be an encoded Ruby object and decoded.
  #
  # If you pass a String key, and the key does not exist on the server, <b>Memcached::NotFound</b> will be raised. If you pass an array of keys, memcached's <tt>multiget</tt> mode will be used, and a hash of key/value pairs will be returned. The hash will contain only the keys that were found.
  #
  # The multiget behavior is subject to change in the future; however, for multiple lookups, it is much faster than normal mode.
  #
  # Note that when you rescue Memcached::NotFound exceptions, you should use a the block rescue syntax instead of the inline syntax. Block rescues are very fast, but inline rescues are very slow.
  #
  def get(keys, decode=true)
    if keys.is_a? Array
      multi_get(keys, decode).first
    else
      single_get(keys, decode).first
    end
  rescue => e
    tries ||= 0
    raise unless tries < options[:exception_retry_limit] && should_retry(e)
    tries += 1
    retry
  end

  # Check if a key exists on the server. It will return nil if the value is found, or raise
  # <tt>Memcached::NotFound</tt> if the key does not exist.
  def exist(key)
    check_return_code(
      Lib.memcached_exist(@struct, key),
      key
    )
  end

  # Gets a key's value from the previous server. Only useful with random distribution.
  def get_from_last(key, decode=true)
    raise ArgumentError, "get_from_last() is not useful unless :random distribution is enabled." unless options[:distribution] == :random
    value, flags, ret = Lib.memcached_get_from_last_rvalue(@struct, key)
    check_return_code(ret, key)
    decode ? @codec.decode(key, value, flags) : value
  end

  ### Information methods

  # Return the server used by a particular key.
  def server_by_key(key)
    ret = Lib.memcached_server_by_key(@struct, key)
    if ret.is_a?(Array)
      check_return_code(ret.last)
      inspect_server(ret.first)
    else
      check_return_code(ret)
    end
  rescue ABadKeyWasProvidedOrCharactersOutOfRange
  end

  # Return a Hash of statistics responses from the set of servers. Each value is an array with one entry for each server, in the same order the servers were defined.
  def stats(subcommand = nil)
    stats = Hash.new([])

    stat_struct, ret = Lib.memcached_stat(@struct, subcommand)
    check_return_code(ret)

    keys, ret = Lib.memcached_stat_get_keys(@struct, stat_struct)
    check_return_code(ret)

    keys.each do |key|
       server_structs.size.times do |index|

         value, ret = Lib.memcached_stat_get_value(
           @struct,
           Lib.memcached_select_stat_at(@struct, stat_struct, index),
           key)
         check_return_code(ret, key)

         value = case value
           when /^\d+\.\d+$/ then value.to_f
           when /^\d+$/ then value.to_i
           else value
         end

         stats[key.to_sym] += [value]
       end
    end

    Lib.memcached_stat_free(@struct, stat_struct)
    stats
  rescue Memcached::SomeErrorsWereReported => _
    e = _.class.new("Error getting stats")
    e.set_backtrace(_.backtrace)
    raise e
  end

  ### Operations helpers

  private

  # Checks the return code from Rlibmemcached against the exception list. Raises the corresponding exception if the return code is not Memcached::Success or Memcached::ActionQueued. Accepts an integer return code and an optional key, for exception messages.
  def check_return_code(ret, key = nil) #:doc:
    if ret == 0 # Lib::MEMCACHED_SUCCESS
    elsif ret == 32 # Lib::MEMCACHED_BUFFERED
    elsif ret == 16
      raise @not_found # Lib::MEMCACHED_NOTFOUND
    elsif ret == 14
      raise @not_stored # Lib::MEMCACHED_NOTSTORED
    else
      reraise(key, ret)
    end
  rescue TypeError
    # Reached if show_backtraces is true
    reraise(key, ret)
  end

  def reraise(key, ret)
    message = "Key #{inspect_keys(key, (detect_failure if ret == Lib::MEMCACHED_SERVER_MARKED_DEAD)).inspect}" if key
    if key.is_a?(String)
      if ret == Lib::MEMCACHED_ERRNO
        if (server = Lib.memcached_server_by_key(@struct, key)).is_a?(Array)
          errno = server.first.cached_errno
          message = "Errno #{errno}: #{ERRNO_HASH[errno].inspect}. #{message}"
        end
      elsif ret == Lib::MEMCACHED_SERVER_ERROR
        if (server = Lib.memcached_server_by_key(@struct, key)).is_a?(Array)
          message = "\"#{server.first.cached_server_error}\". #{message}"
        end
      end
    end
    if EXCEPTIONS[ret]
      raise EXCEPTIONS[ret], message
    else
      raise Memcached::Error, "Unknown return code: #{ret}"
    end
  end

  # Turn an array of keys into a hash of keys to servers.
  def inspect_keys(keys, server = nil)
    Hash[*Array(keys).map do |key|
      [key, server || server_by_key(key)]
    end.flatten]
  end

  # Find which server failed most recently.
  # FIXME Is this still necessary with cached_errno?
  def detect_failure
    time = Time.now
    server = server_structs.detect do |server|
      server.next_retry > time
    end
    inspect_server(server) if server
  end

  # Set the behaviors on the struct from the current options.
  def set_behaviors
    BEHAVIORS.keys.each do |behavior|
      set_behavior(behavior, options[behavior]) if options.key?(behavior)
    end
    # BUG Hash must be last due to the weird Libmemcached multi-behaviors
    set_behavior(:hash, options[:hash])
  end

  # Set the SASL credentials from the current options. If credentials aren't provided, try to get them from the environment.
  def set_credentials
    if options[:credentials]
      check_return_code(
        Lib.memcached_set_sasl_auth_data(@struct, *options[:credentials])
      )
    end
  end

  def is_unix_socket?(server)
    server.type == Lib::MEMCACHED_CONNECTION_UNIX_SOCKET
  end

  # Stringify an opaque server struct
  def inspect_server(server)
    strings = [server.hostname]
    if !is_unix_socket?(server)
      strings << ":#{server.port}"
      strings << ":#{server.weight}" if options[:ketama_weighted]
    end
    strings.join
  end

  def single_get(key, decode)
    value, flags, ret = Lib.memcached_get_rvalue(@struct, key)
    check_return_code(ret, key)
    cas = @struct.result.cas if @support_cas
    value = @codec.decode(key, value, flags) if decode
    [value, flags, cas]
  end

  def multi_get(keys, decode)
    ret = Lib.memcached_mget(@struct, keys)
    check_return_code(ret, keys)

    hash = {}
    flags_and_cas = {} if @support_cas
    value, key, flags, ret = Lib.memcached_fetch_rvalue(@struct)
    while ret != 21 do # Lib::MEMCACHED_END
      if ret == 0 # Lib::MEMCACHED_SUCCESS
        flags_and_cas[key] = [flags, @struct.result.cas] if @support_cas
        hash[key] = decode ? [value, flags] : value
      elsif ret != 16 # Lib::MEMCACHED_NOTFOUND
        check_return_code(ret, key)
      end
      value, key, flags, ret = Lib.memcached_fetch_rvalue(@struct)
    end
    if decode
      hash.each do |key, value_and_flags|
        hash[key] = @codec.decode(key, *value_and_flags)
      end
    end
    [hash, flags_and_cas]
  end

  def multi_cas(hash, ttl, flags_and_cas, encode, tries)
    result = {}
    hash.each do |key, value|
      raw_value = value
      flags, cas = flags_and_cas[key]
      value, flags = @codec.encode(key, value, flags) if encode
      begin
        ret = Lib.memcached_cas(@struct, key, value, ttl, flags, cas)
        if ret == 0 # Lib::MEMCACHED_SUCCESS
          result[key] = raw_value
        elsif ret != 12 && ret != 16 # Lib::MEMCACHED_DATA_EXISTS, Lib::MEMCACHED_NOTFOUND
          check_return_code(ret, key)
        end
      rescue => e
        raise unless tries < options[:exception_retry_limit] && should_retry(e)
        tries += 1
        retry
      end
    end
    result
  end

  def single_cas(key, value, ttl, flags, cas, encode)
    value, flags = @codec.encode(key, value, flags) if encode
    check_return_code(
      Lib.memcached_cas(@struct, key, value, ttl, flags, cas),
      key
    )
  end
end
