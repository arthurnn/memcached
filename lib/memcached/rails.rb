
class Memcached

  alias :get_multi :get #:nodoc:

  # A legacy compatibility wrapper for the Memcached class. It has basic compatibility with the <b>memcache-client</b> API.
  class Rails < ::Memcached
    
    DEFAULTS = {}
         
    # See Memcached#new for details.
    def initialize(*args)
      opts = args.last.is_a?(Hash) ? args.pop : {}
      servers = Array(
        args.any? ? args.unshift : opts.delete(:servers)
      ).flatten.compact

      opts[:prefix_key] ||= opts[:namespace]
      super(servers, DEFAULTS.merge(opts))      
    end
    
    # Wraps Memcached#get so that it doesn't raise. This has the side-effect of preventing you from 
    # storing <tt>nil</tt> values.
    def get(key, raw=false)
      super(key, !raw)
    rescue NotFound
    end

    # Wraps Memcached#cas so that it doesn't raise. Doesn't set anything if no value is present.
    def cas(key, ttl=nil, raw=false, &block)
      super(key, ttl, !raw, &block)
    rescue NotFound    
    end
    
    alias :compare_and_swap :cas    
    
    # Wraps Memcached#get.
    def get_multi(keys, raw=false)
      super(keys, !raw)
    end
    
    # Wraps Memcached#set.
    def set(key, value, ttl=nil, raw=false)
      super(key, value, ttl, !raw)
    end

    # Wraps Memcached#add so that it doesn't raise. 
    def add(key, value, ttl=nil, raw=false)
      super(key, value, ttl, !raw)
      true
    rescue NotStored
      false    
    end
    
    # Wraps Memcached#delete so that it doesn't raise. 
    def delete(key)
      super
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
    
    # Namespace accessor.
    def namespace
      options[:prefix_key]
    end
    
    alias :flush_all :flush

    alias :"[]" :get
    alias :"[]=" :set    
    
  end
end