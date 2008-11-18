
require "#{File.dirname(__FILE__)}/../test_helper"

class RailsTest < Test::Unit::TestCase

  def setup
    @servers = ['127.0.0.1:43042', '127.0.0.1:43043',UNIX_SOCKET_NAME]
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
    result = @cache.get_multi key
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

  def test_bracket_accessors
    @cache[key] = @value
    result = @cache[key]
    assert_equal @value, result
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
    caller.first[/`(.*)'/, 1]
  end
  
end