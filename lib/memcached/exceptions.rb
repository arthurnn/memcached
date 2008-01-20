
class Memcached

  class Error < RuntimeError
  end
  
  class NotImplemented < StandardError
  end

  class << self
    private
    def camelize(string)
      string.downcase.split(' ').map {|s| s.capitalize}.join
    end   
  end
  
  @@exceptions = []
  @@empty_struct = Libmemcached::MemcachedSt.new
  Libmemcached.memcached_create(@@empty_struct)
  
  # Generate exception classes
  Libmemcached::MEMCACHED_MAXIMUM_RETURN.times do |exception_index|    
    description = Libmemcached.memcached_strerror(@@empty_struct, exception_index)
    exception_class = eval("class #{camelize(description)} < Error; self; end")
    @@exceptions << exception_class
  end
  
  # Verify library version
  # XXX Impossible with current libmemcached  
end
