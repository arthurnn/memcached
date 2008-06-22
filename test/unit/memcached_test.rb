
require "#{File.dirname(__FILE__)}/../test_helper"

class MemcachedTest < Test::Unit::TestCase

  def setup
    @servers = ['127.0.0.1:43042', '127.0.0.1:43043']

    # Maximum allowed prefix key size
    @prefix_key = 'prefix_key_' 
    
    @options = {      
      :prefix_key => @prefix_key, 
      :hash => :default,
      :distribution => :modula
    }
    @cache = Memcached.new(@servers, @options)
    
    @nb_options = {
      :prefix_key => @prefix_key, 
      :no_block => true, 
      :buffer_requests => true, 
      :hash => :default
    }
    @nb_cache = Memcached.new(@servers, @nb_options)

    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
    @marshalled_value = Marshal.dump(@value)
  end
  
  def teardown
    ObjectSpace.each_object(Memcached) do |cache|
      cache.destroy rescue Memcached::ClientError
    end
  end
  
  # Initialize

  def test_initialize
    cache = Memcached.new @servers, :prefix_key => 'test'
    assert_equal 'test', cache.options[:prefix_key]
    assert_equal 2, cache.send(:server_structs).size
    assert_equal '127.0.0.1', cache.send(:server_structs).first.hostname
    assert_equal '127.0.0.1', cache.send(:server_structs).last.hostname
    assert_equal 43043, cache.send(:server_structs).last.port
  end
  
  def test_options_are_set
    Memcached::DEFAULTS.merge(@nb_options).each do |key, expected|
      value = @nb_cache.options[key]
      assert(expected == value, "#{key} should be #{expected} but was #{value}")
    end
  end
  
  def test_options_are_frozen
    assert_raise(TypeError) do
      @cache.options[:no_block] = true
    end
  end
  
  def test_destroy
    cache = Memcached.new @servers, :prefix_key => 'test'
    cache.destroy
    assert_raise(Memcached::ClientError) do
      cache.get key
    end
  end
  
  def test_initialize_with_invalid_server_strings
    assert_raise(ArgumentError) { Memcached.new "localhost:43042" }
    assert_raise(ArgumentError) { Memcached.new "127.0.0.1:memcached" }
    assert_raise(ArgumentError) { Memcached.new "127.0.0.1:43043:1" }
  end
  
  def test_initialize_with_invalid_options
    assert_raise(ArgumentError) do 
      Memcached.new @servers, :sort_hosts => true, :distribution => :consistent
    end
  end

  def test_initialize_with_invalid_prefix_key
    assert_raise(ArgumentError) do 
      Memcached.new @servers, :prefix_key => "prefix_key__"
    end
  end
  
  def test_initialize_without_prefix_key
    cache = Memcached.new @servers
    assert_equal nil, cache.options[:prefix_key]
    assert_equal 2, cache.send(:server_structs).size
  end
  
  def test_initialize_negative_behavior
    cache = Memcached.new @servers,
      :buffer_requests => false
    assert_nothing_raised do
      cache.set key, @value
    end
  end
  
  def test_initialize_without_not_found_backtraces
    cache = Memcached.new @servers,
      :show_not_found_backtraces => false
    cache.delete key rescue
    begin
      cache.get key
    rescue Memcached::NotFound => e
      assert e.backtrace.empty?
    end
  end

  def test_initialize_with_not_found_backtraces
    cache = Memcached.new @servers,
      :show_not_found_backtraces => true
    cache.delete key rescue
    begin
      cache.get key
    rescue Memcached::NotFound => e
      assert !e.backtrace.empty?
    end  
  end  
  
  def test_initialize_sort_hosts
    # Original
    cache = Memcached.new(@servers.sort,
      :sort_hosts => false,
      :distribution => :modula
    )
    assert_equal @servers.sort, 
      cache.servers
    cache.destroy

    # Original with sort_hosts
    cache = Memcached.new(@servers.sort,
      :sort_hosts => true,
      :distribution => :modula
    )
    assert_equal @servers.sort, 
      cache.servers
    cache.destroy
    
    # Reversed 
    cache = Memcached.new(@servers.sort.reverse,
      :sort_hosts => false,
      :distribution => :modula
    )
      assert_equal @servers.sort.reverse, 
    cache.servers
    cache.destroy
      
    # Reversed with sort_hosts
    cache = Memcached.new(@servers.sort.reverse,
      :sort_hosts => true,
      :distribution => :modula
    )
    assert_equal @servers.sort, 
      cache.servers
    cache.destroy
  end
  
  def test_initialize_single_server
    cache = Memcached.new '127.0.0.1:43042'
    assert_equal nil, cache.options[:prefix_key]
    assert_equal 1, cache.send(:server_structs).size
  end

  def test_initialize_strange_argument
    assert_raise(ArgumentError) { Memcached.new 1 }
  end
  
  # Get
  
  def test_get
    @cache.set key, @value
    result = @cache.get key
    assert_equal @value, result
  end

  def test_get_nil
    @cache.set key, nil, 0
    result = @cache.get key
    assert_equal nil, result
  end
  
  def test_get_missing
    @cache.delete key rescue nil
    assert_raise(Memcached::NotFound) do
      result = @cache.get key
    end
  end
  
  def test_get_with_prefix_key
    @cache.set key, @value
    assert_equal @value, @cache.get(key)
    
    # No prefix_key specified
    cache = Memcached.new(
      @servers,
      :hash => :default,
      :distribution => :modula
    )
    assert_nothing_raised do    
      assert_equal @value, cache.get("#{@prefix_key}#{key}")
    end
  end

  def test_values_with_null_characters_are_not_truncated
    value = OpenStruct.new(:a => Object.new) # Marshals with a null \000
    @cache.set key, value
    result = @cache.get key, false
    non_wrapped_result = Rlibmemcached.memcached_get(
      @cache.instance_variable_get("@struct"), 
      key
    ).first
    assert result.size > non_wrapped_result.size      
  end  

  def test_get_multi
    @cache.set "#{key}_1", 1
    @cache.set "#{key}_2", 2
    assert_equal({"#{key}_1" => 1, "#{key}_2" => 2},
      @cache.get(["#{key}_1", "#{key}_2"]))
  end
  
  def test_get_multi_missing
    @cache.set "#{key}_1", 1
    @cache.delete "#{key}_2" rescue nil
    @cache.set "#{key}_3", 3
    @cache.delete "#{key}_4" rescue nil    
    assert_equal(
      {"test_get_multi_missing_3"=>3, "test_get_multi_missing_1"=>1}, 
      @cache.get(["#{key}_1", "#{key}_2",  "#{key}_3",  "#{key}_4"])
     )
  end
  
  def test_get_multi_completely_missing
    @cache.delete "#{key}_1" rescue nil
    @cache.delete "#{key}_2" rescue nil    
    assert_equal(
      {},
      @cache.get(["#{key}_1", "#{key}_2"])
     )
  end  

  def test_set_and_get_unmarshalled
    @cache.set key, @value
    result = @cache.get key, false
    assert_equal @marshalled_value, result
  end
  
  def test_get_multi_unmarshalled
    @cache.set "#{key}_1", 1, 0, false
    @cache.set "#{key}_2", 2, 0, false
    assert_equal(
      {"#{key}_1" => "1", "#{key}_2" => "2"},
      @cache.get(["#{key}_1", "#{key}_2"], false)
    )
  end
  
  def test_get_multi_mixed_marshalling
    @cache.set "#{key}_1", 1
    @cache.set "#{key}_2", 2, 0, false
    assert_nothing_raised do
      @cache.get(["#{key}_1", "#{key}_2"], false)
    end
    assert_raise(ArgumentError) do
      @cache.get(["#{key}_1", "#{key}_2"])
    end
  end  

  # Set

  def test_set
    assert_nothing_raised do
      @cache.set(key, @value)
    end
  end
    
  def test_set_expiry
    @cache.set key, @value, 1
    assert_nothing_raised do
      @cache.get key
    end
    sleep(1)
    assert_raise(Memcached::NotFound) do
      @cache.get key
    end
  end
  
  # Delete
  
  def test_delete
    @cache.set key, @value
    @cache.delete key
    assert_raise(Memcached::NotFound) do
      @cache.get key
    end
  end  

  def test_missing_delete
    @cache.delete key rescue nil
    assert_raise(Memcached::NotFound) do
      @cache.delete key
    end
  end
  
  # Flush
  
  def test_flush
    @cache.set key, @value
    assert_equal @value, 
      @cache.get(key)
    @cache.flush
    assert_raise(Memcached::NotFound) do 
      @cache.get key
    end
  end
  
  # Add

  def test_add
    @cache.delete key rescue nil
    @cache.add key, @value
    assert_equal @value, @cache.get(key)
  end

  def test_existing_add
    @cache.set key, @value
    assert_raise(Memcached::NotStored) do
      @cache.add key, @value
    end
  end

  def test_add_expiry
    @cache.delete key rescue nil
    @cache.set key, @value, 1
    assert_nothing_raised do
      @cache.get key
    end
    sleep(1)
    assert_raise(Memcached::NotFound) do
      @cache.get key
    end  
  end

  def test_unmarshalled_add
    @cache.delete key rescue nil
    @cache.add key, @marshalled_value, 0, false
    assert_equal @marshalled_value, @cache.get(key, false)
    assert_equal @value, @cache.get(key)
  end
  
  # Increment and decrement

  def test_increment
    @cache.set key, 10, 0, false
    assert_equal 11, @cache.increment(key)
  end
  
  def test_increment_offset
    @cache.set key, 10, 0, false
    assert_equal 15, @cache.increment(key, 5)
  end

  def test_missing_increment
    @cache.delete key rescue nil
    assert_raise(Memcached::NotFound) do
      @cache.increment key
    end
  end

  def test_decrement
    @cache.set key, 10, 0, false
    assert_equal 9, @cache.decrement(key)
  end
  
  def test_decrement_offset
    @cache.set key, 10, 0, false
    assert_equal 5, @cache.decrement(key, 5)
  end  

  def test_missing_decrement
    @cache.delete key rescue nil
    assert_raise(Memcached::NotFound) do
      @cache.decrement key
    end
  end
  
  # Replace
  
  def test_replace
    @cache.set key, nil
    assert_nothing_raised do
      @cache.replace key, @value
    end
    assert_equal @value, @cache.get(key)
  end

  def test_missing_replace
    @cache.delete key rescue nil
    assert_raise(Memcached::NotStored) do
      @cache.replace key, @value
    end
    assert_raise(Memcached::NotFound) do    
      assert_equal @value, @cache.get(key)    
    end
  end
  
  # Append and prepend

  def test_append
    @cache.set key, "start", 0, false
    assert_nothing_raised do
      @cache.append key, "end"
    end
    assert_equal "startend", @cache.get(key, false)
  end

  def test_missing_append
    @cache.delete key rescue nil
    assert_raise(Memcached::NotStored) do
      @cache.append key, "end"
    end
    assert_raise(Memcached::NotFound) do    
      assert_equal @value, @cache.get(key)    
    end
  end

  def test_prepend
    @cache.set key, "end", 0, false
    assert_nothing_raised do
      @cache.prepend key, "start"
    end
    assert_equal "startend", @cache.get(key, false)
  end

  def test_missing_prepend
    @cache.delete key rescue nil
    assert_raise(Memcached::NotStored) do
      @cache.prepend key, "end"
    end
    assert_raise(Memcached::NotFound) do    
      assert_equal @value, @cache.get(key)    
    end
  end

  def test_cas
    cache = Memcached.new(
      @servers, 
      :prefix_key => @prefix_key, 
      :support_cas => true
    )        
    value2 = OpenStruct.new(:d => 3, :e => 4, :f => GenericClass)

    # Existing set
    cache.set key, @value
    cache.cas(key) do |current|
      assert_equal @value, current
      value2
    end
    assert_equal value2, cache.get(key)
    
    # Existing test without marshalling
    cache.set(key, "foo", 0, false)
    cache.cas(key, 0, false) do |current|
      "#{current}bar"
    end
    assert_equal "foobar", cache.get(key, false)
    
    # Missing set
    cache.delete key
    assert_raises(Memcached::NotFound) do
      cache.cas(key) {}
    end
    
    # Conflicting set
    cache.set key, @value
    assert_raises(Memcached::ConnectionDataExists) do
      cache.cas(key) do |current|
        cache.set key, value2
        current
      end
    end
    
    cache.destroy    
  end
    
  # Error states
  
  def test_key_with_spaces
    key = "i have a space"
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      @cache.set key, @value
    end
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      @cache.get(key)
    end
  end
  
  def test_key_with_null
    key = "with\000null"
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      @cache.set key, @value
    end
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      @cache.get(key)
    end

    # XXX Libmemcached bug
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      response = @cache.get([key])
    end
  end  
  
  def test_key_with_invalid_control_characters
    key = "ch\303\242teau"
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      @cache.set key, @value
    end
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      @cache.get(key)
    end

    # XXX Libmemcached bug
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      response = @cache.get([key])
    end
  end
  
  def test_key_too_long
    key = "x"*251
    assert_raises(Memcached::ClientError) do
      @cache.set key, @value
    end
    assert_raises(Memcached::ClientError) do
      @cache.get(key)
    end
    
    assert_raises(Memcached::ClientError) do
      @cache.get([key])
    end
  end  

  def test_set_object_too_large
    assert_raise(Memcached::ServerError) do
      @cache.set key, "I'm big" * 1000000
    end
  end
  
  # Stats

  def test_stats
    stats = @cache.stats
    assert_equal 2, stats[:pid].size
    assert_instance_of Fixnum, stats[:pid].first
    assert_instance_of String, stats[:version].first
  end
  
  # Clone
  
  def test_clone
    cache = @cache.clone
    assert_equal cache.servers, @cache.servers
    assert_not_equal cache, @cache
    
    # Definitely check that the structs are unlinked
    cache.destroy
    assert_nothing_raised do
      @cache.set key, @value
    end
  end
  
  # Non-blocking IO
  
  def test_buffered_requests_return_value
    cache = Memcached.new @servers,
      :buffer_requests => true
    assert_nothing_raised do
      cache.set key, @value
    end
    ret = Rlibmemcached.memcached_set(
      cache.instance_variable_get("@struct"), 
      key, 
      @marshalled_value, 
      0, 
      Memcached::FLAGS
    )
    assert_equal 31, ret
  end

  def test_no_block_return_value
    assert_nothing_raised do
      @nb_cache.set key, @value
    end
    ret = Rlibmemcached.memcached_set(
      @nb_cache.instance_variable_get("@struct"), 
      key, 
      @marshalled_value, 
      0, 
      Memcached::FLAGS
    )
    assert_equal 31, ret
  end
  
  def test_no_block_missing_delete
    @nb_cache.delete key rescue nil
    assert_nothing_raised do
      @nb_cache.delete key
    end
  end  

  def test_no_block_set_invalid_key
    assert_raises(Memcached::ABadKeyWasProvidedOrCharactersOutOfRange) do
      @nb_cache.set "I'm so bad", @value
    end
  end   

  def test_no_block_set_object_too_large
    assert_nothing_raised do
      @nb_cache.set key, "I'm big" * 1000000
    end
  end
  
  def test_no_block_existing_add
    # Should still raise
    @nb_cache.set key, @value
    assert_raise(Memcached::NotStored) do
      @nb_cache.add key, @value
    end
  end  
  
  # Server removal and consistent hashing
  
  def test_missing_server
    cache = Memcached.new(
      [@servers.last, '127.0.0.1:43041'], # Use a server that isn't running
      :prefix_key => @prefix_key,
      :failover => true,
      :hash => :md5
    )
    
    # Hit second server
    key = 'test_missing_server10'
    assert_raise(Memcached::SystemError) do
      cache.set(key, @value)
      cache.get(key)
    end    

    # Hit first server on retry
    assert_nothing_raised do
      cache.set(key, @value)
      cache.get(key)
    end    
  end
  
  def test_consistent_hashing

    keys = %w(EN6qtgMW n6Oz2W4I ss4A8Brr QShqFLZt Y3hgP9bs CokDD4OD Nd3iTSE1 24vBV4AU H9XBUQs5 E5j8vUq1 AzSh8fva PYBlK2Pi Ke3TgZ4I AyAIYanO oxj8Xhyd eBFnE6Bt yZyTikWQ pwGoU7Pw 2UNDkKRN qMJzkgo2 keFXbQXq pBl2QnIg ApRl3mWY wmalTJW1 TLueug8M wPQL4Qfg uACwus23 nmOk9R6w lwgZJrzJ v1UJtKdG RK629Cra U2UXFRqr d9OQLNl8 KAm1K3m5 Z13gKZ1v tNVai1nT LhpVXuVx pRib1Itj I1oLUob7 Z1nUsd5Q ZOwHehUa aXpFX29U ZsnqxlGz ivQRjOdb mB3iBEAj)
    
    # Five servers
    cache = Memcached.new(
      @servers + ['127.0.0.1:43044', '127.0.0.1:43045', '127.0.0.1:43046'], 
      :prefix_key => @prefix_key
    )        
    
    cache.flush    
    keys.each do |key|
      cache.set(key, @value)
    end 

    # Pull a server
    cache = Memcached.new(
      @servers + ['127.0.0.1:43044', '127.0.0.1:43046'],
      :prefix_key => @prefix_key
    )
    
    failed = 0
    keys.each_with_index do |key, i|
      begin
        cache.get(key)
      rescue Memcached::NotFound
        failed += 1
      end
    end

    assert(failed < keys.size / 3, "#{failed} failed out of #{keys.size}")   
  end

  # Concurrency
  
  def test_thread_contention
    threads = []
    4.times do |index|
      threads << Thread.new do 
        cache = @cache.clone
        assert_nothing_raised do
          cache.set("test_thread_contention_#{index}", index)
        end
        assert_equal index, cache.get("test_thread_contention_#{index}")
      end
    end
    threads.each {|thread| thread.join}
  end
  
  # Memory cleanup
  
  def test_reset
    original_struct = @cache.instance_variable_get("@struct")
    assert_nothing_raised do
      @cache.reset
    end 
    assert_not_equal original_struct, 
      @cache.instance_variable_get("@struct") 
  end
    
  private
  
  def key
    caller.first[/`(.*)'/, 1]
  end
 
end

