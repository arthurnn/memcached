require 'test_helper'

class ClientCASTest < BaseTest

  def test_simple_cas
    cache.set(key, "foo")
    assert_equal "foo", cache.get(key)
    result = cache.cas([key]) do
      { key => @value }
    end
    assert_equal true, result
    assert_equal @value, cache.get(key)
  end

  def test_cas_conflict
    cache.set(key, "foo")
    assert_equal "foo", cache.get(key)
    result = cache.cas([key]) do
      cache.set(key, "bar")
      { key => @value }
    end
    assert_equal false, result
    assert_equal "bar", cache.get(key)
  end

  def test_cas_miss
    result = cache.cas([key]) do
      flunk "CAS block shouldn't be called on miss"
    end
    assert_equal false, result
    assert_nil cache.get(key)
  end

  def test_cas_changed_keys
    cache.set(key, "foo")
    result = cache.cas([key]) do
      { "#{key}:new" => "bar" }
    end
    assert_equal false, result
    assert_nil cache.get("#{key}:new")
  end

  def test_cas_partial_miss
    cache.set(key, "foo")
    result = cache.cas([key, "missing:#{key}"]) do |values|
      assert_equal({ key => "foo" }, values)
      { key => "bar" }
    end
    assert_equal true, result
    assert_equal "bar", cache.get(key)
  end
end
