
require File.expand_path("#{File.dirname(__FILE__)}/../test_helper")

class RailsTest < Test::Unit::TestCase

  def setup
    @servers = ['127.0.0.1:43042', '127.0.0.1:43043', "#{UNIX_SOCKET_NAME}0"]
    @namespace = 'rails_test'
    @cache = Memcached::Rails.new(:servers => @servers, :namespace => @namespace)
    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
    @marshalled_value = Marshal.dump(@value)
  end

  def test_get
    @cache.set key, @value
    result = @cache.get key
    assert_equal @value, result
  end
  
  def test_get_multi
    @cache.set key, @value
    result = @cache.get_multi([key])
    assert_equal(
      {key => @value}, 
      result
    )
  end
  
  def test_delete
    @cache.set key, @value
    assert_nothing_raised do
      @cache.delete key
    end
    assert_nil(@cache.get(key))
  end
  
  def test_delete_missing
    assert_nothing_raised do
      @cache.delete key
      assert_nil(@cache.delete(key))
    end
  end

  def test_delete_with_two_arguments
    assert_nothing_raised do
      @cache.delete(key, 5)
      assert_nil(@cache.get(key))
    end
  end

  def test_bracket_accessors
    @cache[key] = @value
    result = @cache[key]
    assert_equal @value, result
  end
  
  def test_cas
    cache = Memcached::Rails.new(:servers => @servers, :namespace => @namespace, :support_cas => true)
    value2 = OpenStruct.new(:d => 3, :e => 4, :f => GenericClass)

    # Existing set
    cache.set key, @value
    cache.cas(key) do |current|
      assert_equal @value, current
      value2
    end
    assert_equal value2, cache.get(key)

    # Missing set
    cache.delete key
    assert_nothing_raised do
      cache.cas(key) { @called = true }
    end
    assert_nil cache.get(key)
    assert_nil @called

    # Conflicting set
    cache.set key, @value
    assert_raises(Memcached::ConnectionDataExists) do
      cache.cas(key) do |current|
        cache.set key, value2
        current
      end
    end
  end  
  
  def test_get_missing
    @cache.delete key rescue nil
    result = @cache.get key
    assert_equal nil, result
  end    
  
  def test_get_nil
    @cache.set key, nil, 0
    result = @cache.get key
    assert_equal nil, result
  end  
  
  def test_namespace
    assert_equal @namespace, @cache.namespace
  end

  private
  
  def key
    caller.first[/.*[` ](.*)'/, 1] # '
  end
  
end