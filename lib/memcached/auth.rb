class Memcached
  def destroy_credentials
    if options[:credentials] != nil
      check_return_code(Lib.memcached_destroy_sasl_auth_data(@struct))
    end
  end

  def set_credentials
    # If credentials aren't provided, try to get them from the environment
    if options[:credentials] != nil
      username, password = options[:credentials]
      check_return_code(Lib.memcached_set_sasl_auth_data(@struct, username, password))
    end
  end
end

