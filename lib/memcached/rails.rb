
class Memcached

  # A legacy compatibility wrapper for the Memcached class. It has basic compatibility with the <b>memcache-client</b> API.
  class Rails < ::Memcached
    
    DEFAULTS = {:no_block => true}
    
    # See Memcached#new for details.
    def initialize(servers, opts = {})
      super(servers, DEFAULTS.merge(opts))      
    end
    
    # Wraps Memcached#get so that it doesn't raise. This has the side-effect of preventing you from 
    # storing <tt>nil</tt> values.
    def get(*args)
      super
    rescue NotFound 
      nil      
    end
    
    # Namespace accessor.
    def namespace
      @namespace
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