require File.expand_path("#{File.dirname(__FILE__)}/../test_helper")

class MemcachedExperimentalTest < Test::Unit::TestCase

  def setup
    @servers = ['localhost:43042', 'localhost:43043', "#{UNIX_SOCKET_NAME}0"]

    # Maximum allowed prefix key size for :hash_with_prefix_key_key => false
    @prefix_key = 'prefix_key_'

    @options = {
      :prefix_key => @prefix_key,
      :hash => :default,
      :distribution => :modula}
    @cache = Memcached.new(@servers, @options)

    @experimental_options = {
      :prefix_key => @prefix_key,
      :hash => :default,
      :distribution => :modula,
      :experimental_features => true}
    @experimental_cache = Memcached.new(@servers, @experimental_options)

    @experimental_cas_options = {
      :prefix_key => @prefix_key,
      :support_cas => true,
      :experimental_features => true}
    @experimental_cas_cache = Memcached.new(@servers, @experimental_cas_options)

    @experimental_binary_protocol_options = {
      :prefix_key => @prefix_key,
      :hash => :default,
      :distribution => :modula,
      :experimental_features => true,
      # binary_protocol does not work -- test_get, test_get, test_append, and test_missing_append will fail when it is set to true.
      :binary_protocol => true}
    @experimental_binary_protocol_cache = Memcached.new(@servers, @experimental_binary_protocol_options)

    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
  end

  def server_touch_capable
    @experimental_binary_protocol_cache.set(key, 'touchval')
    @experimental_binary_protocol_cache.touch(key)
    yield
  rescue Memcached::ProtocolError
    # Skip tests when the server does not support the experimental touch command
  end

  # Touch command

  def test_touch_capability
    server_touch_capable do
      @experimental_binary_protocol_cache.set(key, "value", 10)
      assert_nothing_raised do
        @experimental_binary_protocol_cache.touch(key, 20)
      end
      assert_raises(Memcached::ActionNotSupported) do
        @experimental_cache.touch(key, 10)
      end
    end
  end

  def test_touch_missing_key
    server_touch_capable do
      @experimental_binary_protocol_cache.delete key rescue nil
      assert_raises(Memcached::NotFound) do
        @experimental_binary_protocol_cache.touch(key, 10)
      end
    end
  end

  def test_touch
    server_touch_capable do
      @experimental_binary_protocol_cache.set key, @value, 2
      assert_equal @value, @experimental_binary_protocol_cache.get(key)
      sleep(1)
      assert_equal @value, @experimental_binary_protocol_cache.get(key)
      @experimental_binary_protocol_cache.touch(key, 3)
      sleep(2)
      assert_equal @value, @experimental_binary_protocol_cache.get(key)
      sleep(2)
      assert_raises(Memcached::NotFound) do
        @experimental_binary_protocol_cache.get(key)
      end
    end
  end

  # Memory cleanup

  def test_reset
    original_struct = @cache.instance_variable_get("@struct")
    assert_nothing_raised do
      @cache.reset
    end
    assert_not_equal original_struct,
      @cache.instance_variable_get("@struct")
  end

  private

  def key
    caller.first[/.*[` ](.*)'/, 1] # '
  end

  def stub_server(port)
    socket = TCPServer.new('127.0.0.1', port)
    Thread.new { socket.accept }
    socket
  end

end
