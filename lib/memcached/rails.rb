
class Memcached
  class Rails < ::Memcached
    
    DEFAULTS = {:no_block => true}
    
    def initialize(servers, opts = {})
      super(servers, DEFAULTS.merge(opts))      
    end
    
    def get(*args)
      super
    rescue NotFound 
      nil      
    end
    
  end
end