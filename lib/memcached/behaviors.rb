module Memcached
  module Behaviors
    def set_behavior(behavior, value)
      behavior = convert_behavior(behavior)
      value = convert_value(behavior, value)
      _set_behavior(behavior, value)
    rescue Memcached::Deprecated
      warn "Behavior #{behavior_string} is deprecated, and won't work anymore."
    end

    def get_behavior(behavior)
      _get_behavior(convert_behavior(behavior))
    end

    def set_behaviors(hash)
      hash.each { |key, value| set_behavior(key, value) } if hash
    end

    protected
    CONVERSION_FACTORS = {
      MEMCACHED_BEHAVIOR_RCV_TIMEOUT => 1_000_000,
      MEMCACHED_BEHAVIOR_SND_TIMEOUT => 1_000_000,
      MEMCACHED_BEHAVIOR_POLL_TIMEOUT => 1_000,
      MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT => 1_000
    }

    VALUE_PREFIXES = {
      MEMCACHED_BEHAVIOR_HASH => "MEMCACHED_HASH_",
      MEMCACHED_BEHAVIOR_DISTRIBUTION => "MEMCACHED_DISTRIBUTION_",
    }

    def convert_behavior(behavior)
      case behavior
      when Numeric
        behavior
      else
        lookup_constant('MEMCACHED_BEHAVIOR_', behavior)
      end
    end

    def convert_value(behavior, value)
      case value
      when Symbol, String
        lookup_constant(VALUE_PREFIXES[behavior], value)
      when Numeric
        value * (CONVERSION_FACTORS[behavior] || 1)
      else
        value
      end
    end

    def lookup_constant(prefix, ct)
      raise ArgumentError unless prefix
      ct = ct.to_s.upcase
      ct = prefix + ct unless ct.start_with? prefix
      Behaviors.const_get(ct)
    rescue NameError, ArgumentError
      raise ArgumentError, "Invalid constant #{ct.inspect}"
    end
  end
end
