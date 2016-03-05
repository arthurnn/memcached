require 'test_helper'

require 'ostruct'
require 'securerandom'

class ClientTest < Minitest::Test
  def setup
    @cache = Memcached::Client.new#(@servers, @options)
  end

  def test_flush
    @cache.set "foo", "bar"
    assert_equal "bar", @cache.get("foo")
    @cache.flush
    assert_nil @cache.get("foo")
  end
end
