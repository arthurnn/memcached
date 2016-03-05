require 'test_helper'

class ClientDeleteTest < Minitest::Test

  def setup
    @cache = Memcached::Client.new#(@servers, @options)
    @cache.flush
    @cache.set key, @value = "bar"
  end

  def key
    caller.first[/.*[` ](.*)'/, 1]
  end

  def test_simple_delete
    @cache.delete key
    assert_nil @cache.get(key)
  end

#  def test_binary_delete
#    @binary_protocol_cache.set key, @value
#    @binary_protocol_cache.delete key
#    assert_raise(Memcached::NotFound) do
#      @binary_protocol_cache.get key
#    end
#  end

  def test_missing_delete
    refute @cache.delete(key)
  end
end
