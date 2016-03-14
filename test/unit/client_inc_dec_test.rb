require 'test_helper'

class ClientIncDecTest < BaseTest

  def test_increment_simple
    cache.set key, "10", raw: true
    assert_equal 11, cache.increment(key)
  end

  def test_increment_binary
    binary_protocol_cache.set key, "10", raw: true
    assert_equal 11, binary_protocol_cache.increment(key)
  end

  def test_increment_offset
    cache.set key, "10", raw: true
    assert_equal 15, cache.increment(key, 5)
  end

  def test_missing_increment
    assert_equal(-1, cache.increment(key))
  end

  def test_decrement_simple
    cache.set key, "10", raw: true
    assert_equal 9, cache.decrement(key)
  end

  def test_decrement_binary
    binary_protocol_cache.set key, "10", raw: true
    assert_equal 9, binary_protocol_cache.decrement(key)
  end

  def test_decrement_offset
    cache.set key, "10", raw: true
    assert_equal 5, cache.decrement(key, 5)
  end

  def test_decrement_with_negative
    cache.set key, "10", raw: true
    assert_equal 0, cache.decrement(key, 15)
  end

  def test_missing_decrement
    assert_equal(-1, cache.decrement(key))
  end
end
