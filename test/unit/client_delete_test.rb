require 'test_helper'

class ClientDeleteTest < BaseTest

  def setup
    super
    cache.set key, @value = "bar"
  end

  def test_simple_delete
    cache.delete key
    assert_nil cache.get(key)
  end

  def test_binary_delete
    binary_protocol_cache.set key, @value
    refute_nil binary_protocol_cache.get(key)

    binary_protocol_cache.delete key

    assert_nil binary_protocol_cache.get(key)
  end

  def test_missing_delete
    refute cache.delete(key)
  end
end
