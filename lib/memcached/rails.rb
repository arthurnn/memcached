require 'memcached'

class Memcached

  (instance_methods - NilClass.instance_methods).each do |method_name|
    eval("alias :'#{method_name}_orig' :'#{method_name}'")
  end

  # A legacy compatibility wrapper for the Memcached class. It has basic compatibility with the <b>memcache-client</b> API and Rails 3.2. (Note that ActiveSupport::Duration objects are supported, but not recommended, as ttl parameters. Using Fixnum ttls, such as provided by time_constants.gem, is much faster.)
  class Rails < ::Memcached

    DEFAULTS = {
      :logger => nil,
      :string_return_types => false
    }

    attr_reader :logger

    alias :servers= :set_servers

    # See Memcached#new for details.
    def initialize(*args)
      opts = args.last.is_a?(Hash) ? args.pop : {}
      servers = Array(
        args.any? ? args.unshift : opts.delete(:servers)
      ).flatten.compact

      opts[:prefix_key] = opts.delete(:namespace) if opts[:namespace]
      opts[:prefix_delimiter] = opts.delete(:namespace_separator) if opts[:namespace_separator]

      @logger = opts.delete(:logger)
      @string_return_types = opts.delete(:string_return_types)

      logger.info { "memcached #{VERSION} #{servers.inspect}" } if logger
      super(servers, opts)
    end

    def logger=(logger)
      @logger = logger
    end

    def namespace
       @options[:prefix_key]
    end

    # Check if there are any servers defined?
    def active?
      servers.any?
    end

    def log_exception(e)
      logger.warn("memcached error: #{e.class}: #{e.message}") if logger
      false
    end

    # Wraps Memcached#get so that it doesn't raise. This has the side-effect of preventing you from
    # storing <tt>nil</tt> values.
    def get(key, raw=false)
      super(key, !raw)
    rescue NotFound
    rescue Error => e
      log_exception e
    end

    # Alternative to #get. Accepts a key and an optional options hash supporting the single option
    # :raw.
    def read(key, options = nil)
      if options
        get(key, options[:raw])
      else
        get(key)
      end
    rescue NotFound
    rescue Error => e
      log_exception e
    end

    # Returns whether the key exists, even if the value is nil.
    def exist?(key, options = {})
      exist(key)
      true
    rescue NotFound
      false
    rescue Error => e
      log_exception e
    end

    # Wraps Memcached#cas so that it doesn't raise. Doesn't set anything if no value is present.
    def cas(key, ttl=@default_ttl, raw=false, &block)
      super(key, ttl, !raw, &block)
      true
    rescue TypeError => e
      # Maybe we got an ActiveSupport::Duration
      ttl = ttl.value and retry rescue raise e
    rescue NotFound, ConnectionDataExists
      false
    rescue Error => e
      log_exception e
    end

    alias :compare_and_swap :cas

    # Wraps Memcached#get.
    def get_multi(keys, raw=false)
      get_orig(keys, !raw)
    rescue NotFound
    rescue Error => e
      log_exception e
    end

    # Wraps Memcached#set.
    def set(key, value, ttl=@default_ttl, raw=false)
      super(key, value, ttl, !raw)
      true
    rescue TypeError => e
      # Maybe we got an ActiveSupport::Duration
      ttl = ttl.value and retry rescue raise e
    rescue NotStored
      false
    rescue Error => e
      log_exception e
    end

    # Alternative to #set. Accepts a key, value, and an optional options hash supporting the
    # options :raw and :ttl.
    def write(key, value, options = nil)
      value = value.to_s if options && options[:raw]
      ttl = options ? (options[:ttl] || options[:expires_in] || @default_ttl) : @default_ttl
      raw = options ? options[:raw] : nil
      if options && options[:unless_exist]
        add(key, value, ttl, raw)
      else
        set(key, value, ttl, raw)
      end
    end

    def fetch(key, options = nil)
      result = read(key, options)
      if result.nil?
        if block_given?
          result = yield
          write(key, result, options)
          result
        else
          result
        end
      else
        result
      end
    end

    # Wraps Memcached#add so that it doesn't raise.
    def add(key, value, ttl=@default_ttl, raw=false)
      super(key, value, ttl, !raw)
      @string_return_types ? "STORED\r\n" : true
    rescue TypeError => e
      # Maybe we got an ActiveSupport::Duration
      ttl = ttl.value and retry rescue raise e
    rescue NotStored
    rescue Error => e
      log_exception e
      @string_return_types? "NOT STORED\r\n" : false
    end

    # Wraps Memcached#delete so that it doesn't raise.
    def delete(key, options = nil)
      super(key)
      true
    rescue NotFound
      false
    rescue Error => e
      log_exception e
    end

    # Wraps Memcached#incr so that it doesn't raise.
    def incr(*args)
      super
    rescue NotFound
    rescue Error => e
      log_exception e
    end

    # Wraps Memcached#decr so that it doesn't raise.
    def decr(*args)
      super
    rescue NotFound
    rescue Error => e
      log_exception e
    end

    # Wraps Memcached#append so that it doesn't raise.
    def append(*args)
      super
    rescue NotStored
    rescue Error => e
      log_exception e
    end

    # Wraps Memcached#prepend so that it doesn't raise.
    def prepend(*args)
      super
    rescue NotStored
    rescue Error => e
      log_exception e
    end

    alias :flush_all :flush
    alias :clear :flush

    alias :"[]" :get
    alias :"[]=" :set

    def read_multi(*keys)
      return {} if keys.empty?
      get_multi(keys)
    end

    # Return an array of server objects.
    def servers
      server_structs.each do |server|
        def server.alive?
          next_retry <= Time.now
        end
      end
    end

    # Wraps Memcached#set_servers to convert server objects to strings.
    def set_servers(servers)
      servers = Array(servers)
      servers.map! do |server|
        server.is_a?(String) ? server : inspect_server(server)
      end
      super
    end

    def increment(name, amount = 1, options = nil)
      response = super(name, amount)
      response ? response.to_i : nil
    rescue
      nil
    end

    def decrement(name, amount = 1, options = nil)
      response = super(name, amount)
      response ? response.to_i : nil
    rescue
      nil
    end
  end
end
