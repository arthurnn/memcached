
class Memcached

#:stopdoc:

  def self.load_constants(prefix, hash = {}, offset = 0)
    Rlibmemcached.constants.grep(/^#{prefix}/).each do |const_name|
      hash[const_name[prefix.length..-1].downcase.to_sym] = Rlibmemcached.const_get(const_name) + offset
    end
    hash
  end

  BEHAVIORS = load_constants("MEMCACHED_BEHAVIOR_")

  BEHAVIOR_VALUES = {
    false => 0, 
    true => 1
  }

  HASH_VALUES = {}
  BEHAVIOR_VALUES.merge!(load_constants("MEMCACHED_HASH_", HASH_VALUES, 2))

  DISTRIBUTION_VALUES = {}
  BEHAVIOR_VALUES.merge!(load_constants("MEMCACHED_DISTRIBUTION_", DISTRIBUTION_VALUES, 2))

#:startdoc:

  private
  
  # Set a behavior option for this Memcached instance. Accepts a Symbol <tt>behavior</tt> and either <tt>true</tt>, <tt>false</tt>, or a Symbol for <tt>value</tt>. Arguments are validated and converted into integers for the struct setter method.
  def set_behavior(behavior, value) #:doc:
    raise ArgumentError, "No setting #{behavior.inspect}" unless b_id = BEHAVIORS[behavior]    
    raise ArgumentError, "No setting value #{value.inspect}" unless v_id = BEHAVIOR_VALUES[value]
    
    # Scoped validations
    msg =  "Invalid setting value #{value.inspect} for #{behavior.inspect}" 
    if behavior == :hash
      raise ArgumentError, msg unless HASH_VALUES[value]
    elsif behavior == :distribution
      raise ArgumentError, msg unless DISTRIBUTION_VALUES[value]
    end
    # STDERR.puts "Setting #{behavior}:#{b_id} => #{value}:#{v_id}"
    
    unless value == false
      # XXX Setting false still turns on the behavior; maybe a Rlibmemcached bug
      Rlibmemcached.memcached_behavior_set(@struct, b_id, v_id)
    end
    
  end  
      
end