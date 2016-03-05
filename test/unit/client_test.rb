require 'test_helper'

class ClientTest < BaseTest
  def test_flush
    cache.set "foo", "bar"
    assert_equal "bar", cache.get("foo")
    cache.flush
    assert_nil cache.get("foo")
  end
end
