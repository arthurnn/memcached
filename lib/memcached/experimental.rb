class Memcached
  module Experimental
  
    def get_len(bytes, keys)
      if keys.is_a? ::Array
        # Multi get
        ret = Lib.memcached_mget_len(@struct, keys, bytes);
        check_return_code(ret, keys)
  
        hash = {}
        keys.each do
          value, key, flags, ret = Lib.memcached_fetch_rvalue(@struct)
          break if ret == Lib::MEMCACHED_END
          if ret != Lib::MEMCACHED_NOTFOUND
            check_return_code(ret, key)
            # Assign the value
            hash[key] = value
          end
        end
        hash
      else
        # Single get_len
        value, flags, ret = Lib.memcached_get_len_rvalue(@struct, keys, bytes)
        check_return_code(ret, keys)
        value
      end
    rescue => e
      tries ||= 0
      raise unless tries < options[:exception_retry_limit] && should_retry(e)
      tries += 1
      retry
    end
    
  end
end