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

  def server_get_len_capable
    begin
      value = "foobar"
      @experimental_cache.set key, value, 0, false
      @experimental_cache.get_len 1, key
      yield
    rescue Memcached::ProtocolError
      # Skip tests when the server does not support the experimental get_len command.
    end
  end

  def server_touch_capable
    @experimental_binary_protocol_cache.set(key, 'touchval')
    @experimental_binary_protocol_cache.touch(key)
    yield
  rescue Memcached::ProtocolError
    # Skip tests when the server does not support the experimental touch command
  end

  def test_get_len
    server_get_len_capable do
      value = "foobar"
      @experimental_cache.set key, value, 0, false
      result = @experimental_cache.get_len 1, key
      assert_equal result.size, 1
      assert_equal result, value[0..0]

      result = @experimental_cache.get_len 2, key
      assert_equal result.size, 2
      assert_equal result, value[0..1]

      result = @experimental_cache.get_len 5, key
      assert_equal result.size, 5
      assert_equal result, value[0..4]

      result = @experimental_cache.get_len 6, key
      assert_equal result.size, 6
      assert_equal result, value

      result = @experimental_cache.get_len 32, key
      assert_equal result.size, 6
      assert_equal result, value
    end
  end

  def test_get_len_0_failure
    server_get_len_capable do
      value = "Test that we cannot get 0 bytes with a get_len call."
      @experimental_cache.set key, value, 0, false
      assert_raises(Memcached::Failure) do
        result = @experimental_cache.get_len 0, key
      end
    end
  end

  def test_get_len_large
    server_get_len_capable do
      value = "Test that we can get the first 20 bytes of a string"
      @experimental_cache.set key, value, 0, false
      result = @experimental_cache.get_len 20, key
      assert_equal result.size, 20
      assert_equal result, value[0..19]
    end
  end

  def test_get_len_packed
    server_get_len_capable do
      value = [1, 2, 3, 4].pack("Q*")
      @experimental_cache.set key, value, 0, false
      result = @experimental_cache.get_len 8, key
      assert_equal [1], result.unpack("Q*")
    end
  end

  # get_len is not supported when using the binary protocol.
  # Make sure the single get variant fails appropriately.
  def test_get_len_binary
    server_get_len_capable do
      @experimental_binary_protocol_cache.set key, @value
      assert_raises(Memcached::ActionNotSupported) do
        result = @experimental_binary_protocol_cache.get_len 2, key
      end
    end
  end

  # Retrieve the first 64 bits of the values for multiple keys.
  def test_get_len_multi_packed
    server_get_len_capable do
      key_1 = "get_len_1"
      value_1 = [1, 2, 3].pack("Q*")
      key_2 = "get_len_missing"
      key_3 = "get_len_2"
      value_3 = [5, 6, 4].pack("Q*")
      keys = [key_1, key_2, key_3]
      @experimental_cache.set key_1, value_1, 0, false
      @experimental_cache.set key_3, value_3, 0, false
      assert_equal(
        {key_1=>value_1[0..7], key_3=>value_3[0..7]},
        @experimental_cache.get_len(8, keys)
      )
    end
  end

  # Test that the entire value is passed back when the length specified
  # is larger than any of the values (e.g., 32 in the case below).
  def test_get_len_multi_packed_full
    server_get_len_capable do
      key_1 = "get_len_1"
      value_1 = [1, 2, 3].pack("Q*")
      key_2 = "get_len_missing"
      key_3 = "get_len_2"
      value_3 = [5, 6, 4].pack("Q*")
      keys = [key_1, key_2, key_3]
      @experimental_cache.set key_1, value_1, 0, false
      @experimental_cache.set key_3, value_3, 0, false
      assert_equal(
        {key_1=>value_1, key_3=>value_3},
        @experimental_cache.get_len(32, keys)
      )
    end
  end

  # get_len is not supported when using the binary protocol.
  # Test that the multi get variant fails appropriately.
  def test_get_len_multi_packed_binary
    server_get_len_capable do
      key_1 = "get_len_1"
      value_1 = [1, 2, 3].pack("Q*")
      key_2 = "get_len_2"
      value_2 = [5, 6, 4].pack("Q*")
      keys = [key_1, key_2]
      @experimental_binary_protocol_cache.set key_1, value_1, 0, false
      @experimental_binary_protocol_cache.set key_2, value_2, 0, false
      assert_raises(Memcached::ActionNotSupported) do
        result = @experimental_binary_protocol_cache.get_len 2, keys
      end
    end
  end

  def test_get_len_multi_completely_missing
    server_get_len_capable do
      @experimental_cache.delete "#{key}_1" rescue nil
      @experimental_cache.delete "#{key}_2" rescue nil
      assert_equal(
        {},
        @experimental_cache.get_len(1, ["#{key}_1", "#{key}_2"])
       )
    end
  end

  def test_get_len_failure
    server_get_len_capable do
      value = "Test that we cannot use get_len without setting the :experimental_features config."
      assert_raises(NoMethodError) do
        result = @cache.get_len 10, key
      end
    end
  end

  def test_cas
    server_get_len_capable do
      # Get the first three chars of the value back.
      @experimental_cas_cache.set(key, "foo_bar", 0, false)
      assert_equal "foo", @experimental_cas_cache.get_len(3, key)
      assert_equal "foo_bar", @experimental_cas_cache.get_len(7, key)
    end
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
