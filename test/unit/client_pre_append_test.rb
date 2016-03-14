require 'test_helper'

class ClientPreAppendTest < BaseTest
  def test_append_simple
    cache.set key, "start", raw: true

    assert cache.append(key, "end")
    assert_equal "startend", cache.get(key, raw: true)
  end

  def test_append_binary
    binary_protocol_cache.set key, "start", raw: true
    binary_protocol_cache.append key, "end"

    assert_equal "startend", binary_protocol_cache.get(key, raw: true)
  end

  def test_missing_append_simple
    refute cache.append(key, "end")
    assert_nil cache.get(key)
  end

  def test_missing_append_binary
    refute binary_protocol_cache.append(key, "end")

    assert_nil binary_protocol_cache.get(key)
  end

  def test_prepend
    cache.set key, "end", raw: true
    assert cache.prepend(key, "start")
    assert_equal "startend", cache.get(key, raw: true)
  end

  def test_missing_prepend
    refute cache.prepend(key, "end")
    assert_nil cache.get(key)
  end
end
