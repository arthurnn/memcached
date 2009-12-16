class Memcached
  
  def maybe_destroy_credentials
    if @has_credentials
      Lib.memcached_destroy_sasl_auth_data(@struct)
    end
  end
    
  def set_credentials(credentials=nil)
    maybe_destroy_credentials
    if credentials != nil 
      print "setting credentials to #{credentials}\n"
      result = Lib.memcached_set_sasl_auth_data(@struct, credentials[0], credentials[1])
      if result == Lib.const_get("MEMCACHED_SUCCESS")
        @has_credentials = true
        print "Successfully set auth data.\n"
      elsif result == Lib.const_get("MEMCACHED_FAILURE")
        print "Got MEMCACHED_FAILURE trying to set auth data.\n"
      elsif result == Lib.const_get("MEMCACHED_MEMORY_ALLOCATION_FAILURE")
        print "Got MEMCACHED_MEMORY_ALLOCATION_FAILURE trying to set auth data.\n"
      else
        print "Got unexpected result trying to set auth data.\n"
      end
    end
  end
end

