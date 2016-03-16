require 'test_helper'

class ClientSetTest < BaseTest

  def test_simple_set
    assert cache.set(key, @value)
  end

  def test_binary_set
    assert binary_protocol_cache.set(key, @value)
  end

#  def test_udp_set
#    assert @udp_cache.set(key, @value)
#  end

  def test_set_expiry
    cache.set key, @value, ttl: 1

    assert_equal @value, cache.get(key)
    sleep(1.02)
    assert_nil cache.get(key)
    assert_raises(TypeError) do
      cache.set key, @value, ttl: Time.now
    end
  end

  def test_set_with_default_ttl
    cache = Memcached::Client.new(@servers, ttl: 1)
    cache.set key, @value
    assert_equal @value, cache.get(key)

    sleep(1.02)

    assert_nil cache.get(key)
  end

  def test_set_coerces_string_type
    refute cache.set(nil, @value)

    assert_raises(TypeError) do
      cache.set 1, @value
    end
  end

#  def disabled_test_set_retry_on_client_error
#    # FIXME Test passes, but Mocha doesn't restore the original method properly
#    Rlibmemcached.stubs(:memcached_set).raises(Memcached::ClientError)
#    Rlibmemcached.stubs(:memcached_set).returns(0)
#
#    assert_nothing_raised do
#      cache.set(key, @value)
#    end
#  end

  def test_set_with_server_timeout
    socket = stub_server 43047
    cache = Memcached::Client.new("localhost:43047", :timeout => 0.5)
    assert_raises(Memcached::Timeout) do
      cache.set key, "v"
    end

    cache = Memcached::Client.new("localhost:43047", :poll_timeout => 0.001, :rcv_timeout => 0.5)
    assert_raises(Memcached::Timeout) do
      cache.set key, "v"
    end

    cache = Memcached::Client.new("localhost:43047", :poll_timeout => 0.25, :rcv_timeout => 0.25)
    assert_raises(Memcached::Timeout) do
      cache.set key, "v"
    end
  ensure
    socket.close if socket
  end

  def test_set_with_no_block_server_timeout
    socket = stub_server 43048
    cache = Memcached::Client.new("localhost:43048", :no_block => true, :timeout => 0.25)

    assert_raises(Memcached::Timeout) do
      cache.set key, "v"
    end

    cache = Memcached::Client.new("localhost:43048", :no_block => true, :poll_timeout => 0.25, :rcv_timeout => 0.001)
    assert_raises(Memcached::Timeout) do
      cache.set key, "v"
    end

    cache = Memcached::Client.new("localhost:43048", :no_block => true,
      :poll_timeout => 0.001,
      :rcv_timeout => 0.25 # No affect in no-block mode
    )
    assert_raises(Memcached::Timeout) do
      cache.set key, "v"
    end
  ensure
    socket.close if socket
  end
end
