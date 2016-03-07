require 'test_helper'

class ClientTest < BaseTest

  # Flush

  def test_flush
    cache.set "foo", "bar"
    assert_equal "bar", cache.get("foo")
    cache.flush
    assert_nil cache.get("foo")
  end

  # Exist

  def test_missing_exist
    refute cache.exist(key)
  end

  def test_exist
    cache.set key, @value
    assert cache.exist(key)
  end

#  def test_missing_exist_binary
#    assert_raise(Memcached::NotFound) do
#      @binary_protocol_cache.exist key
#    end
#  end
#
#  def test_exist_binary
#    @binary_protocol_cache.set key, @value
#    assert_nil @binary_protocol_cache.exist(key)
  #  end

  # Replace

  def test_replace
    cache.set key, nil
    cache.replace key, @value
    assert_equal @value, cache.get(key)
  end

  def test_missing_replace
    refute cache.replace(key, @value)
    assert_nil cache.get(key)
  end
end
