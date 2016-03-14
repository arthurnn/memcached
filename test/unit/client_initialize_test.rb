require 'test_helper'

class ClientInitializeTest < BaseTest

  def test_initialize_without_servers
    client = Memcached::Client.new
    assert_equal [[:tcp, "localhost", 11211]], client.servers
    assert_equal "localhost", client.connection.servers.first.hostname
    assert_equal 11211, client.connection.servers.first.port
  end

  def test_initialize_with_multiple_servers
    @servers = ['localhost:43042', 'localhost:43043']
    client = Memcached::Client.new @servers
    assert_equal [[:tcp, "localhost", 43042], [:tcp, "localhost", 43043]], client.servers
  end

  def test_initialize_with_multiple_servers_and_socket
    @servers = ['localhost:43042', 'localhost:43043']
    client = Memcached::Client.new(@servers + ['/tmp/memcached0'])
    assert_equal [[:tcp, "localhost", 43042], [:tcp, "localhost", 43043], [:socket, '/tmp/memcached0']], client.servers

    servers = client.connection.servers
    assert_equal [["localhost", 43042], ["localhost", 43043], ['/tmp/memcached0', 0]], servers.map { |s| [s.hostname, s.port] }
  end

  def test_initialize_with_ip_addresses
    cache = Memcached::Client.new ['127.0.0.1:43042', '127.0.0.1:43043']
    assert_equal '127.0.0.1', cache.connection.servers.first.hostname
    assert_equal '127.0.0.1', cache.connection.servers.last.hostname
  end

  def test_initialize_without_port
    cache = Memcached::Client.new ['localhost']
    assert_equal 'localhost', cache.connection.servers.first.hostname
    assert_equal 11211, cache.connection.servers.first.port
  end

## TODO
#  def test_initialize_with_ports_and_weights
#    cache = Memcached.new ['localhost:43042:2', 'localhost:43043:10']
#    assert_equal 2, cache.send(:server_structs).first.weight
#    assert_equal 43043, cache.send(:server_structs).last.port
#    assert_equal 10, cache.send(:server_structs).last.weight
#  end

  def test_initialize_with_hostname_only
    addresses = (1..8).map { |i| "app-cache-%02d" % i }
    cache = Memcached::Client.new(addresses)
    addresses.each_with_index do |address, index|
      assert_equal address, cache.connection.servers[index].hostname
      assert_equal 11211, cache.connection.servers[index].port
    end
  end

  def test_initialize_with_ip_address_and_options
    cache = Memcached::Client.new '127.0.0.1:43042', :ketama_weighted => false
    assert_equal '127.0.0.1', cache.connection.servers.first.hostname
    assert_equal false, cache.options[:ketama_weighted]
  end

#  def test_options_are_set
#    Memcached::DEFAULTS.merge(@noblock_options).each do |key, expected|
#      value = @noblock_cache.options[key]
#      unless key == :rcv_timeout or key == :poll_timeout or key == :prefix_key or key == :ketama_weighted or key == :snd_timeout
#        assert(expected == value, "#{key} should be #{expected} but was #{value}")
#      end
#    end
#  end

  def test_options_are_frozen
    assert_raises(TypeError, RuntimeError) do
      cache.options[:no_block] = true
    end
  end

  def test_behaviors_are_set
    conn = cache.connection

    refute conn.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_NO_BLOCK)
    conn.set_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_NO_BLOCK, true)
    assert conn.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_NO_BLOCK)
  end

  def test_initialize_with_invalid_server_strings
    assert_raises(ArgumentError) { Memcached::Client.new ":43042" }
    assert_raises(ArgumentError) { Memcached::Client.new "localhost:memcached" }
    assert_raises(ArgumentError) { Memcached::Client.new "local host:43043:1" }
  end

  def test_initialize_with_sort_host_and_consistent_distribution
    assert_raises(ArgumentError) do
      Memcached::Client.new @servers, :sort_hosts => true, :distribution => :consistent
    end
  end

  def test_initialize_with_invalid_options
    assert_raises(ArgumentError) do
      client = Memcached::Client.new @servers, foo: true
      client.connection
    end
  end

  def test_initialize_with_invalid_prefix_key
    assert_raises(Memcached::KeyTooBig) do
      client = Memcached::Client.new @servers, :prefix_key => "x" * 128
      client.connection
    end
  end

  def test_set_namespace
    cache = Memcached::Client.new @servers, :prefix_key => "foo"
    assert_equal "foo", cache.namespace

    cache.namespace = "bar"
    assert_equal "bar", cache.namespace
  end

  def test_set_prefix_key_to_empty_string
    cache = Memcached::Client.new @servers, :prefix_key => "foo"
    assert_raises(Memcached::InvalidArgument) do
      cache.namespace = ""
    end
    assert_equal "foo", cache.namespace
  end

  def test_set_prefix_key_to_nil
    cache = Memcached::Client.new @servers, :prefix_key => "foo"
    cache.namespace = nil
    assert_equal nil, cache.namespace
  end

  def test_initialize_negative_behavior
    cache = Memcached::Client.new @servers, :buffer_requests => false
    cache.set key, @value
  end

  def test_initialize_sort_hosts
    # Original
    cache = Memcached::Client.new(@servers.sort,
      :sort_hosts => false,
      :distribution => :modula
    )
    assert_equal @servers.sort, cache.connection.servers.map(&:to_s)

    # Original with sort_hosts
    cache = Memcached::Client.new(@servers.sort,
      :sort_hosts => true,
      :distribution => :modula
    )
    assert_equal @servers.sort, cache.connection.servers.map(&:to_s)

    # Reversed
    cache = Memcached::Client.new(@servers.sort.reverse,
      :sort_hosts => false,
      :distribution => :modula
    )
    assert_equal @servers.sort.reverse, cache.connection.servers.map(&:to_s)

    # Reversed with sort_hosts
    cache = Memcached::Client.new(@servers.sort.reverse,
      :sort_hosts => true,
      :distribution => :modula
    )
    assert_equal @servers.sort, cache.connection.servers.map(&:to_s)
  end

  def test_initialize_strange_argument
    assert_raises(ArgumentError) { Memcached::Client.new 1 }
  end
end
