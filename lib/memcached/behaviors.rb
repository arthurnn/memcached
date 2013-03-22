
class Memcached

#:stopdoc:

  def self.load_constants(prefix, hash = {})
    Lib.constants.grep(/^#{prefix}/).each do |const_name|
      hash[const_name[prefix.length..-1].downcase.to_sym] = Lib.const_get(const_name)
    end
    hash
  end

  BEHAVIORS = load_constants("MEMCACHED_BEHAVIOR_")

  BEHAVIOR_VALUES = {
    false => 0,
    true => 1
  }

  HASH_VALUES = {}
  BEHAVIOR_VALUES.merge!(load_constants("MEMCACHED_HASH_", HASH_VALUES))

  DISTRIBUTION_VALUES = {}
  BEHAVIOR_VALUES.merge!(load_constants("MEMCACHED_DISTRIBUTION_", DISTRIBUTION_VALUES))

  DIRECT_VALUE_BEHAVIORS = [:retry_timeout, :connect_timeout, :rcv_timeout, :socket_recv_size, :poll_timeout, :socket_send_size, :server_failure_limit, :snd_timeout, :poll_max_retries]

  CONVERSION_FACTORS = {
    :rcv_timeout => 1_000_000,
    :snd_timeout => 1_000_000,
    :poll_timeout => 1_000,
    :connect_timeout => 1_000
  }

#:startdoc:

  private

  # Set a behavior option for this Memcached instance. Accepts a Symbol <tt>behavior</tt> and either <tt>true</tt>, <tt>false</tt>, or a Symbol for <tt>value</tt>. Arguments are validated and converted into integers for the struct setter method.
  def set_behavior(behavior, value) #:doc:
    raise ArgumentError, "No behavior #{behavior.inspect}" unless b_id = BEHAVIORS[behavior]

    # Scoped validations; annoying
    msg =  "Invalid behavior value #{value.inspect} for #{behavior.inspect}"
    case behavior
      when :hash then raise(ArgumentError, msg) unless HASH_VALUES[value]
      when :distribution then raise(ArgumentError, msg) unless DISTRIBUTION_VALUES[value]
      when *DIRECT_VALUE_BEHAVIORS then raise(ArgumentError, msg) unless value.is_a?(Numeric) and value >= 0
      else
        raise(ArgumentError, msg) unless BEHAVIOR_VALUES[value]
    end

    lib_value = BEHAVIOR_VALUES[value] || (value * (CONVERSION_FACTORS[behavior] || 1)).to_i
    #STDERR.puts "Setting #{behavior}:#{b_id} => #{value} (#{lib_value})"
    Lib.memcached_behavior_set(@struct, b_id, lib_value)
    #STDERR.puts " -> set to #{get_behavior(behavior).inspect}"
  end

  # Get a behavior value for this Memcached instance. Accepts a Symbol.
  def get_behavior(behavior)
    raise ArgumentError, "No behavior #{behavior.inspect}" unless b_id = BEHAVIORS[behavior]
    value = Lib.memcached_behavior_get(@struct, b_id)

    if BEHAVIOR_VALUES.invert.has_key?(value)
      # False, nil are valid values so we can not rely on direct lookups
      case behavior
        # Scoped values; still annoying
        when :hash then HASH_VALUES.invert[value]
        when :distribution then DISTRIBUTION_VALUES.invert[value]
        else
          value
      end
    else
      value
    end
  end

end
