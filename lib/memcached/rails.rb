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

      opts[:prefix_key] = opts[:namespace] if opts[:namespace]
      opts[:prefix_delimiter] = opts[:namespace_separator] if opts[:namespace_separator]

      @logger = opts[:logger]
      @string_return_types = opts[:string_return_types]

      logger.info { "memcached #{VERSION} #{servers.inspect}" } if logger
      super(servers, opts)
    end

    def logger=(logger)
      @logger = logger
    end

    # Check if there are any servers defined?
    def active?
      servers.any?
    end

    # Wraps Memcached#get so that it doesn't raise. This has the side-effect of preventing you from
    # storing <tt>nil</tt> values.
    def get(key, raw=false)
      super(key, !raw)
    rescue NotFound
    end

    # Alternative to #get. Accepts a key and an optional options hash supporting the single option
    # :raw.
    def read(key, options = {})
      get(key, options[:raw])
    end

    # Returns whether the key exists, even if the value is nil.
    def exist?(key, options = {})
      get_orig(key, options)
      true
    rescue NotFound
      false
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
    end

    alias :compare_and_swap :cas

    # Wraps Memcached#get.
    def get_multi(keys, raw=false)
      get_orig(keys, !raw)
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
    end

    # Alternative to #set. Accepts a key, value, and an optional options hash supporting the
    # options :raw and :ttl.
    def write(key, value, options = {})
      set(key, value, options[:ttl] || options[:expires_in] || @default_ttl, options[:raw])
    end

    def fetch(key, options={})
      result = read(key, options)
      if result.nil?
        result = yield
        write(key, result, options)
        result
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
      @string_return_types? "NOT STORED\r\n" : false
    end

    # Wraps Memcached#delete so that it doesn't raise.
    def delete(key, expiry=0)
      super(key)
    rescue NotFound
    end

    # Wraps Memcached#incr so that it doesn't raise.
    def incr(*args)
      super
    rescue NotFound
    end

    # Wraps Memcached#decr so that it doesn't raise.
    def decr(*args)
      super
    rescue NotFound
    end

    # Wraps Memcached#append so that it doesn't raise.
    def append(*args)
      super
    rescue NotStored
    end

    # Wraps Memcached#prepend so that it doesn't raise.
    def prepend(*args)
      super
    rescue NotStored
    end

    alias :flush_all :flush

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
  end
end
