
class Memcached #:nodoc:

  # A Rails compatibility wrapper for the Memcached class.
  class Rails < ::Memcached
    
    DEFAULTS = {:no_block => true}
    
    # See Memcached#new for details.
    def initialize(servers, opts = {})
      super(servers, DEFAULTS.merge(opts))      
    end
    
    # Wraps Memcached#get so that it doesn't raise. This prevents you from 
    # setting <tt>nil</tt> values.
    def get(*args)
      super
    rescue NotFound 
      nil      
    end

    # Alias for get.
    def [](key)
      get key
    end

    # Alias for set.
    def []=(key, value)
      set key, value
    end
    
  end
end