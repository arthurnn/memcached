class Memcached

  (instance_methods - NilClass.instance_methods).each do |method_name|
    eval("alias :'#{method_name}_orig' :'#{method_name}'")
  end

  # A legacy compatibility wrapper for the Memcached class. It has basic compatibility with the <b>memcache-client</b> API.
  class Rails < ::Memcached

    DEFAULTS[:logger] = nil

    attr_reader :logger

    alias :servers= :set_servers

    # See Memcached#new for details.
    def initialize(*args)
      opts = args.last.is_a?(Hash) ? args.pop : {}
      servers = Array(
        args.any? ? args.unshift : opts.delete(:servers)
      ).flatten.compact

      opts[:prefix_key] ||= opts[:namespace]
      @logger = opts[:logger]
      logger.info { "memcached #{VERSION} #{servers.inspect}" } if logger
      super(servers, opts)
    end

    def logger=(logger)
      @logger = logger
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

    # Wraps Memcached#cas so that it doesn't raise. Doesn't set anything if no value is present.
    def cas(key, ttl=@default_ttl, raw=false, &block)
      super(key, ttl, !raw, &block)
    rescue NotFound
    end

    alias :compare_and_swap :cas

    # Wraps Memcached#get.
    def get_multi(keys, raw=false)
      get_orig(keys, !raw)
    end

    # Wraps Memcached#set.
    def set(key, value, ttl=@default_ttl, raw=false)
      super(key, value, ttl, !raw)
    end

    # Alternative to #set. Accepts a key, value, and an optional options hash supporting the 
    # options :raw and :ttl.
    def write(key, value, options = {})
      set(key, value, options[:ttl] || @default_ttl, options[:raw])
      true
    end

    # Wraps Memcached#add so that it doesn't raise.
    def add(key, value, ttl=@default_ttl, raw=false)
      super(key, value, ttl, !raw)
      true
    rescue NotStored
      false
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

  end
end