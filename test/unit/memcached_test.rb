
require "#{File.dirname(__FILE__)}/../test_helper"

class MemcachedTest < Test::Unit::TestCase

  def setup
    @servers = ['127.0.0.1:43042', '127.0.0.1:43043']
    @cache = Memcached.new(
      @servers, 
      :namespace => 'class_test_namespace'
    )
    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
    @marshalled_value = Marshal.dump(@value)
  end

  def test_initialize
    cache = Memcached.new @servers, :namespace => 'test'
    assert_equal 'test', cache.options[:namespace]
    assert_equal 2, cache.send(:server_structs).size
    assert_equal '127.0.0.1', cache.send(:server_structs).first.hostname
    assert_equal '127.0.0.1', cache.send(:server_structs).last.hostname
    assert_equal 43043, cache.send(:server_structs).last.port
  end
  
  def test_initialize_with_invalid_server_strings
    assert_raise(ArgumentError) { Memcached.new "localhost:43042" }
    assert_raise(ArgumentError) { Memcached.new "127.0.0.1:memcached" }
    assert_raise(ArgumentError) { Memcached.new "127.0.0.1:43043:1" }
  end

  def test_initialize_without_namespace
    cache = Memcached.new @servers
    assert_equal nil, cache.options[:namespace]
    assert_equal 2, cache.send(:server_structs).size
  end
  
  def test_initialize_positive_behavior
    cache = Memcached.new @servers,
      :buffer_requests => true
    assert_raise(Memcached::ActionQueued) do
      cache.set key, @value
    end
  end

  def test_initialize_negative_behavior
    cache = Memcached.new @servers,
      :buffer_requests => false
    assert_nothing_raised do
      cache.set key, @value
    end
  end

  def test_initialize_single_server
    cache = Memcached.new '127.0.0.1:43042'
    assert_equal nil, cache.options[:namespace]
    assert_equal 1, cache.send(:server_structs).size
  end

  def test_initialize_strange_argument
    assert_raise(ArgumentError) { Memcached.new 1 }
  end

  def test_get
    @cache.set key, @value
    result = @cache.get key
    assert_equal @value, result
  end
  
  def test_get_with_namespace
    @cache.set key, @value
    result = @cache.get key, false
    direct_result = Libmemcached.memcached_get(
      @cache.instance_variable_get("@struct"), 
      "#{@cache.options[:namespace]}#{key}"
    ).first  
    assert_equal result, direct_result
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

  def test_truncation_issue_is_covered
    value = OpenStruct.new(:a => Object.new) # Marshals with a null \000
    @cache.set key, value
    result = @cache.get key, false
    non_wrapped_result = Libmemcached.memcached_get(
      @cache.instance_variable_get("@struct"), 
      "#{@cache.options[:namespace]}#{key}"
    ).first
    assert result.size > non_wrapped_result.size      
  end  

  def test_get_invalid_key
    assert_raise(Memcached::ClientError) { @cache.get(key * 100) }
    assert_raise(Memcached::ClientError) { @cache.get "I'm so bad" }
  end

  def test_get_multi
    @cache.set "#{key}_1", 1
    @cache.set "#{key}_2", 2
    assert_equal [1, 2], 
      @cache.get(["#{key}_1", "#{key}_2"])
  end

  def test_set_and_get_unmarshalled
    @cache.set key, @value
    result = @cache.get key, false
    assert_equal @marshalled_value, result
  end

  def test_set
    assert_nothing_raised do
      @cache.set(key, @value)
    end
  end
  
  def test_set_invalid_key
    assert_raise(Memcached::ProtocolError) do
      @cache.set "I'm so bad", @value
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

  def test_set_object_too_large
    assert_raise(Memcached::ServerError) do
      @cache.set key, "I'm big" * 1000000
    end
  end
  
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

  def test_increment
    @cache.set key, 10, 0, false
    assert_equal 11, @cache.increment(key)
  end
  
  def test_increment_offset
    @cache.set key, 10, 0, false
    assert_equal 15, @cache.increment(key, 5)
  end

  def test_missing_increment
    # XXX Fails due to libmemcached bug
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
    # XXX Fails due to libmemcached bug
    @cache.delete key rescue nil
    assert_raise(Memcached::NotFound) do
      @cache.decrement key
    end
  end
  
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
    # XXX Not implemented
  end

  def test_stats
    stats = @cache.stats
    assert_equal 2, stats[:pid].size
    assert_instance_of Fixnum, stats[:pid].first
    assert_instance_of String, stats[:version].first
  end
  
  def test_clone
    cache = @cache.clone
    assert_equal cache.servers, @cache.servers
    assert_not_equal cache, @cache
  end

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
  
  private
  
  def key
    caller.first[/`(.*)'/, 1]
  end

end

