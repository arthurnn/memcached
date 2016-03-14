require 'test_helper'

class ClientAddTest < BaseTest

  def test_add
    cache.add key, @value
    assert_equal @value, cache.get(key)
  end

  def test_existing_add
    cache.set key, @value
    refute cache.add(key, @value)
  end

  def test_add_expiry
    cache.set key, @value, ttl: 1
    assert cache.get(key)

    sleep(1.02)
    assert_nil cache.get(key)
  end

  def test_unmarshalled_add
    cache.add key, @marshalled_value, raw: true
    assert_equal @marshalled_value, cache.get(key, raw: true)
    assert_equal @value, cache.get(key)
  end
end
