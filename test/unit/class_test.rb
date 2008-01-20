
require "#{File.dirname(__FILE__)}/../test_helper"

require 'ruby-debug' if ENV['DEBUG']

class ClassTest < Test::Unit::TestCase

  def setup
    @cache = Memcached.new(
      ['127.0.0.1:43042', '127.0.0.1:43043'], 
      :namespace => 'test'
    )
    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
    @marshalled_value = Marshal.dump(@value)
  end
  
  #  def teardown
  #    @cache.send(:free)
  #  end

  def test_initialize
    cache = Memcached.new ['127.0.0.1:43042', '127.0.0.1:43043'], :namespace => 'test'
    assert_equal 'test', cache.namespace
    assert_equal 2, cache.servers.size
    assert_equal '127.0.0.1', cache.servers.first.hostname
    assert_equal '127.0.0.1', cache.servers.last.hostname
    assert_equal 43043, cache.servers.last.port
  end
  
  def test_initialize_with_invalid_server_strings
    assert_raise(ArgumentError) { Memcached.new "localhost:43042" }
    assert_raise(ArgumentError) { Memcached.new "127.0.0.1:memcached" }
    assert_raise(ArgumentError) { Memcached.new "127.0.0.1:43043:1" }
  end

  def test_initialize_without_namespace
    cache = Memcached.new ['127.0.0.1:43042', '127.0.0.1:43043']
    assert_equal nil, cache.namespace
    assert_equal 2, cache.servers.size
  end
  
  def test_intialize_alternative_hashing_scheme
  end

  def test_initialize_single_server
    cache = Memcached.new '127.0.0.1:43042'
    assert_equal nil, cache.namespace
    assert_equal 1, cache.servers.size
  end

  def test_initialize_strange_argument
    assert_raise(ArgumentError) { Memcached.new 1 }
  end

  def test_get_missing
    assert_raise(Memcached::NotFound) do
      result = @cache.get 'test_get_missing'
    end
  end

  def test_get
    @cache.set 'test_get', @value, 0
    result = @cache.get 'test_get'
    assert_equal @value, result
  end
  
  def test_truncation_issue_is_covered
    value = OpenStruct.new(:a => 1, :b => 2, :c => Object.new) # Marshals with a null \000
    @cache.set 'test_get', value, 0
    result = @cache.get 'test_get', false
    non_wrapped_result = Libmemcached.memcached_get(
      @cache.instance_variable_get("@struct"), 
      'test_get'
    ).first
    assert result.size > non_wrapped_result.size      
  end  

  def test_get_invalid_key
    assert_raise(Memcached::ClientError) { @cache.get('key' * 100) }
    assert_raise(Memcached::ClientError) { @cache.get "I'm so bad" }
  end

  def test_get_multi
    @cache.set 'test_get_multi_1', 1
    @cache.set 'test_get_multi_2', 2
    assert_equal [1, 2], 
      @cache.get(['test_get_multi_1', 'test_get_multi_1'])
  end

  def test_set_and_get_unmarshalled
    @cache.set 'test_set_and_get_unmarshalled', @value
    result = @cache.get 'test_set_and_get_unmarshalled', false
    assert_equal @marshalled_value, result
  end

  def test_set
    assert_equal true, @cache.set('test_set', @value)
  end
  
  def test_set_invalid_key
    assert_raise(Memcached::ClientError) do
      @cache.set "I'm so bad", @value
    end
  end
  
  def test_set_expiry
    @cache.set 'test_set_expiry', @value, 1
    assert_nothing_raised do
      @cache.get 'test_set_expiry'
    end
    sleep(1)
    assert_raise(Memcached::NotFound) do
      @cache.get 'test_set_expiry'
    end
  end

  def test_set_object_too_large
    assert_raise(Memcached::ServerError) do
      @cache.set 'test_set_object_too_large', "I'm big" * 1000000
    end
  end
  
  def test_delete
    @cache.set 'test_delete', @value
    @cache.delete 'test_delete'
    assert_raise(Memcached::NotFound) do
      @cache.get 'test_delete'
    end
  end  

  def test_successful_add
    @cache.delete 'test_successful_add' rescue nil
    @cache.add 'test_successful_add', @value
  end

  def test_failing_add
    @cache.delete 'test_failing_add' rescue nil
    @cache.add 'test_failing_add', @value
  end

  def test_add_expiry
    @cache.delete 'test_add_expiry' rescue nil
    @cache.set 'test_add_expiry', @value, 1
    assert_nothing_raised do
      @cache.get 'test_add_expiry'
    end
    sleep(1)
    assert_raise(Memcached::NotFound) do
      @cache.get 'test_add_expiry'
    end  
  end

  def test_unmarshalled_add
    @cache.delete 'test_unmarshalled_add' rescue nil
    @cache.add 'test_unmarshalled_add', @marshalled_value, 0, false
  end

  def test_increment
    @cache.set 'test_increment', 10, 0, false
    assert_equal 11, @cache.increment('test_increment')
  end

  def test_missing_increment
    @cache.delete 'test_missing_increment' rescue nil
    assert_raise(Memcached::NotFound) do
      @cache.increment 'test_missing_increment'
    end
  end

  def test_decrement
    @cache.set 'test_decrement', 10, 0, false
    assert_equal 9, @cache.decrement('test_decrement')
  end

  def test_missing_decrement
    @cache.delete 'test_missing_decrement' rescue nil
    assert_raise(Memcached::NotFound) do
      @cache.decrement 'test_missing_decrement'
    end
  end
  
  def test_replace
  end

  def test_append
  end

  def test_prepend
  end

  def test_cas
  end

  def test_stats
  end
  
  #  def test_freed_struct_cannot_be_used
  #    @cache.send(:free)
  #    assert_raise(NoMethodError) do
  #      @cache.get 'test_freed_struct_cannot_be_used'
  #    end
  #  end

  def test_thread_contention
  end

end

