
class Memcached

  alias :get_multi :get #:nodoc:

  # A legacy compatibility wrapper for the Memcached class. It has basic compatibility with the <b>memcache-client</b> API.
  class Rails < ::Memcached
    
    DEFAULTS = {}
         
    # See Memcached#new for details.
    def initialize(servers, opts = {})
      super(servers, DEFAULTS.merge(opts))      
    end
    
    # Wraps Memcached#get so that it doesn't raise. This has the side-effect of preventing you from 
    # storing <tt>nil</tt> values.
    def get(key, raw = false)
      super(key, !raw)
    rescue NotFound
    end
    
    # Wraps Memcached#get with multiple arguments.
    def get_multi(*keys)
      super(keys)
    end
    
    # Wraps Memcached#set.
    def set(key, value, ttl = 0, raw = false)
      super(key, value, ttl, !raw)
    end
    
    # Wraps Memcached#delete so that it doesn't raise. 
    def delete(key)
      super(key)
    rescue NotFound
    end
    
    # Namespace accessor.
    def namespace
      options[:prefix_key]
    end

    # Alias for get.
    def [](key)
      get key
    end        

    # Alias for Memcached#set.
    def []=(key, value)
      set key, value
    end
    
  end
end