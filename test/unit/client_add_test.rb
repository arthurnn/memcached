require 'test_helper'
require 'ostruct'

class ClientAddTest < Minitest::Test

  def setup
    @cache = Memcached::Client.new#(@servers, @options)
    @cache.flush

    @value = OpenStruct.new(a: 1, b: 2, c: self.class)
    @marshalled_value = Marshal.dump(@value)
  end

  def key
    caller.first[/.*[` ](.*)'/, 1]
  end

  def test_add
    @cache.add key, @value
    assert_equal @value, @cache.get(key)
  end

  def test_existing_add
    @cache.set key, @value
    refute @cache.add(key, @value)
  end

  def test_add_expiry
    @cache.set key, @value, ttl: 1
    assert @cache.get(key)

    sleep(1.02)
    assert_nil @cache.get(key)
  end

  def test_unmarshalled_add
    @cache.add key, @marshalled_value, raw: true
    assert_equal @marshalled_value, @cache.get(key, raw: true)
    assert_equal @value, @cache.get(key)
  end
end
