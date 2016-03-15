module Memcached
  module Behaviors

    CONVERSION_FACTORS = {
      MEMCACHED_BEHAVIOR_RCV_TIMEOUT => 1_000_000,
      MEMCACHED_BEHAVIOR_SND_TIMEOUT => 1_000_000,
      MEMCACHED_BEHAVIOR_POLL_TIMEOUT => 1_000,
      MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT => 1_000
    }

    BASE_VALUES = {
      MEMCACHED_BEHAVIOR_HASH => "MEMCACHED_HASH_%s",
      MEMCACHED_BEHAVIOR_DISTRIBUTION => "MEMCACHED_DISTRIBUTION_%s",
    }

    def lookup_value(behavior, value)
      base_value = BASE_VALUES[behavior]
      raise ArgumentError unless base_value
      value = base_value % value.to_s.upcase
      Behaviors.const_get(value)
    end

    def set_behaviors(hash)
      return unless hash
      hash.each do |key, value|
        behavior_string = "MEMCACHED_BEHAVIOR_#{key.to_s.upcase}"
        begin
          behavior = Behaviors.const_get(behavior_string)
        rescue NameError
          raise ArgumentError, "No behavior #{behavior_string.inspect}"
        end

        if value.is_a? Symbol
          begin
            value = lookup_value(behavior, value)
          rescue NameError, ArgumentError
            msg =  "Invalid behavior value #{value.inspect} for #{behavior_string.inspect}"
            raise ArgumentError, msg
          end
        end

        if value.is_a? Numeric
          value *= (CONVERSION_FACTORS[behavior] || 1)
        end

        begin
          set_behavior(behavior, value)
        rescue Memcached::Deprecated
          warn "Behavior #{behavior_string} is deprecated, and won't work anymore."
        end
      end
    end
  end
end
