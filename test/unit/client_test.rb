require 'test_helper'

class ClientTest < BaseTest

  # Flush

  def test_flush
    cache.set "foo", "bar"
    assert_equal "bar", cache.get("foo")
    cache.flush
    assert_nil cache.get("foo")
  end

  # Exist

  def test_missing_exist
    refute cache.exist(key)
  end

  def test_exist
    cache.set key, @value
    assert cache.exist(key)
  end

  def test_missing_exist_binary
    refute binary_protocol_cache.exist(key)
  end

  def test_exist_binary
    binary_protocol_cache.set key, @value
    assert binary_protocol_cache.exist(key)
  end

  # Replace

  def test_replace
    cache.set key, nil
    cache.replace key, @value
    assert_equal @value, cache.get(key)
  end

  def test_missing_replace
    refute cache.replace(key, @value)
    assert_nil cache.get(key)
  end

  # Error states

  def test_key_with_spaces
    key = "i have a space"
    assert_raises(Memcached::BadKeyProvided) do
      cache.set key, @value
    end
    assert_raises(Memcached::BadKeyProvided) do
      cache.get(key)
    end
  end

  def test_key_with_null
    key = "with\000null"
    assert_raises(Memcached::BadKeyProvided) do
      cache.set key, @value
    end
    assert_raises(Memcached::BadKeyProvided) do
      cache.get(key)
    end
    assert_raises(Memcached::BadKeyProvided) do
      cache.get_multi([key])
    end
  end

  def test_key_with_invalid_control_characters
    key = "ch\303\242teau"
    assert_raises(Memcached::BadKeyProvided) do
      cache.set key, @value
    end
    assert_raises(Memcached::BadKeyProvided) do
      cache.get(key)
    end
    assert_raises(Memcached::BadKeyProvided) do
      cache.get_multi([key])
    end
  end

  def test_key_too_long
    key = "x"*251
    assert_raises(Memcached::BadKeyProvided) do
      cache.set key, @value
    end
    assert_raises(Memcached::BadKeyProvided) do
      cache.get(key)
    end
    assert_raises(Memcached::BadKeyProvided) do
      cache.get_multi([key])
    end
  end

  def test_server_error_message
    err = assert_raises(Memcached::E2big) do
      cache.set key, "I'm big" * 1000000
    end
    assert_equal "ITEM TOO BIG", err.message
  end

  def test_clone_with_connection_loaded
    @cache = cache
    cache = @cache.clone
    assert_equal cache.config, @cache.config
    refute_equal cache, @cache

    # Definitely check that the connections are unlinked
    refute_equal @cache.connection.object_id, cache.connection.object_id

    # normal set, should work
    @cache.set key, @value
  end

  def test_clone_with_connected_client
    @cache = cache
    @cache.set key, @value

    cache = @cache.clone
    assert_equal cache.config, @cache.config
    refute_equal cache, @cache

    refute_nil cache.connection.servers
    # Definitely check that the connections are unlinked
    refute_equal @cache.connection.object_id, cache.connection.object_id

    # normal set, should work
    @cache.set key, @value
  end

  def test_thread_contention
    @cache = cache

    threads = []
    4.times do |index|
      threads << Thread.new do
        cache = @cache.clone
        cache.set("test_thread_contention_#{index}", "value#{index}")
        assert_equal "value#{index}", cache.get("test_thread_contention_#{index}")
      end
    end
    threads.each { |thread| thread.join }
  end

  def test_consistent_hashing
    keys = %w(EN6qtgMW n6Oz2W4I ss4A8Brr QShqFLZt Y3hgP9bs CokDD4OD Nd3iTSE1 24vBV4AU H9XBUQs5 E5j8vUq1 AzSh8fva PYBlK2Pi Ke3TgZ4I AyAIYanO oxj8Xhyd eBFnE6Bt yZyTikWQ pwGoU7Pw 2UNDkKRN qMJzkgo2 keFXbQXq pBl2QnIg ApRl3mWY wmalTJW1 TLueug8M wPQL4Qfg uACwus23 nmOk9R6w lwgZJrzJ v1UJtKdG RK629Cra U2UXFRqr d9OQLNl8 KAm1K3m5 Z13gKZ1v tNVai1nT LhpVXuVx pRib1Itj I1oLUob7 Z1nUsd5Q ZOwHehUa aXpFX29U ZsnqxlGz ivQRjOdb mB3iBEAj)

    # Five servers
    cache = Memcached::Client.new(
      @servers + ['localhost:43044', 'localhost:43045', 'localhost:43046'],
      :prefix_key => @prefix_key
    )

    cache.flush
    keys.each do |key|
      cache.set(key, @value)
    end

    # Pull a server
    cache = Memcached::Client.new(
      @servers + ['localhost:43044', 'localhost:43046'],
      :prefix_key => @prefix_key
    )

    failed = 0
    keys.each_with_index do |key, i|
      unless cache.get(key)
        failed += 1
      end
    end

    assert(failed < keys.size / 3, "#{failed} failed out of #{keys.size}")
  end

  module YAMLCodec
    extend self

    FLAG = Memcached::Client::FLAG_ENCODED

    def encode(key, value, flags)
      [ YAML.dump(value), flags | FLAG ]
    end

    def decode(key, value, flags)
      if (flags & FLAG) != 0
        YAML.load(value)
      else
        value
      end
    end
  end

  def test_custom_codec
    cache = Memcached::Client.new(@servers, codec: YAMLCodec)
    cache.set("yaml", :symbol)
    assert_equal :symbol, cache.get("yaml")
    assert_equal YAML.dump(:symbol), cache.get("yaml", raw: true)
  end

  # Touch command

  def test_touch_missing_key
    refute cache.touch(key, 10)
  end

  def test_touch
    cache.set key, @value, ttl: 2
    assert_equal @value, cache.get(key)
    sleep(1)
    assert_equal @value, cache.get(key)
    cache.touch(key, 3)
    sleep(2)
    assert_equal @value, cache.get(key)
    sleep(2.02)
    refute cache.get(key)
  end

  def test_reset
    con = cache.connection
    cache.reset
    refute_equal cache.connection, con
  end

  # Namespace

  def test_namespace
    cache.namespace = nil
    cache.set('ns:key', @value)

    cache.namespace = 'ns:'

    assert_equal 'ns:', cache.namespace
    assert_equal @value, cache.get('key')
  end

  def test_namespace_after_reset
    cache.namespace = 'ns:'
    cache.set('key', @value)

    cache.reset

    assert_equal 'ns:', cache.namespace
    assert_equal @value, cache.get('key')
  end
end
