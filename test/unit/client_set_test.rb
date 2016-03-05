require 'test_helper'

require 'ostruct'
require 'securerandom'

class ClientSetTest < Minitest::Test

  def setup
    @cache = Memcached::Client.new#(@servers, @options)
    @cache.flush

    @value = OpenStruct.new(a: 1, b: 2, c: self.class)
    @marshalled_value = Marshal.dump(@value)
  end

  def key
    caller.first[/.*[` ](.*)'/, 1]
  end

  def test_simple_set
    assert @cache.set(key, @value)
  end

#  def test_binary_set
#    assert @binary_protocol_cache.set(key, @value)
#  end
#
#  def test_udp_set
#    assert @udp_cache.set(key, @value)
#  end

  def test_set_expiry
    @cache.set key, @value, ttl: 1

    assert_equal @value, @cache.get(key)
    sleep(2)
    assert_nil @cache.get(key)
    assert_raises(TypeError) do
      @cache.set key, @value, ttl: Time.now
    end
  end

  def test_set_with_default_ttl
    cache = Memcached::Client.new(@servers, ttl: 1)
    cache.set key, @value
    assert_equal @value, cache.get(key)

    sleep(2)

    assert_nil cache.get(key)
  end

  def test_set_coerces_string_type
    refute @cache.set(nil, @value)

    assert_raises(TypeError) do
      @cache.set 1, @value
    end
  end

#  def disabled_test_set_retry_on_client_error
#    # FIXME Test passes, but Mocha doesn't restore the original method properly
#    Rlibmemcached.stubs(:memcached_set).raises(Memcached::ClientError)
#    Rlibmemcached.stubs(:memcached_set).returns(0)
#
#    assert_nothing_raised do
#      @cache.set(key, @value)
#    end
#  end
end
