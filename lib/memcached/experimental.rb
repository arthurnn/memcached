class Memcached
  module Experimental
    # TOUCH is used to set a new expiration time for an existing item
    def touch(key, ttl=@default_ttl)
      check_return_code(
        Lib.memcached_touch(@struct, key, ttl),
        key
      )
    rescue => e
      tries ||= 0
      raise unless should_retry(e, tries)
      tries += 1
      retry
    end
  end
end
