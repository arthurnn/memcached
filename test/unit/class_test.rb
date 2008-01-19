
require "#{File.dirname(__FILE__)}/../test_helper"

class ClassTest < Test::Unit::TestCase

  def setup
    @cache = Memcached.new(
      ['localhost:43042', '127.0.0.1:43043'], 
      :namespace => 'test'
    )
  end

  def test_initialize_compatible
    cache = Memcached.new ['localhost:43042', '127.0.0.1:43043'],
     :namespace => 'test'

    assert_equal 'test', cache.namespace
    assert_equal false, cache.servers.empty?
  end

  def test_initialize_compatible_no_hash
    cache = Memcached.new ['localhost:43042', '127.0.0.1:43043']

    assert_equal nil, cache.namespace
    assert_equal false, cache.servers.empty?
  end

  def test_initialize_compatible_one_server
    cache = Memcached.new 'localhost:43042'

    assert_equal nil, cache.namespace
    assert_equal false, cache.servers.empty?
  end

  def test_initialize_compatible_bad_arg
    e = assert_raise ArgumentError do
      cache = Memcached.new Object.new
    end

    assert_equal 'first argument must be Array, Hash or String', e.message
  end

  def test_initialize_too_many_args
    assert_raises ArgumentError do
      Memcached.new 1, 2, 3
    end
  end

  def test_decr
    server = FakeServer.new
    server.socket.data.write "5\r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server

    value = @cache.decr 'key'

    assert_equal "decr test:key 1\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal 5, value
  end

  def test_decr_not_found
    server = FakeServer.new
    server.socket.data.write "NOT_FOUND\r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server

    value = @cache.decr 'key'

    assert_equal "decr test:key 1\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal nil, value
  end

  def test_decr_space_padding
    server = FakeServer.new
    server.socket.data.write "5 \r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server

    value = @cache.decr 'key'

    assert_equal "decr test:key 1\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal 5, value
  end

  def test_get
    util_setup_fake_server

    value = @cache.get 'key'

    assert_equal "get test:key\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal '0123456789', value
  end

  def test_get_bad_key
    util_setup_fake_server
    assert_raise ArgumentError do @cache.get 'k y' end

    util_setup_fake_server
    assert_raise ArgumentError do @cache.get 'k' * 250 end
  end

  def test_get_cache_get_IOError
    socket = Object.new
    def socket.write(arg) raise IOError, 'some io error'; end
    server = FakeServer.new socket

    @cache.servers = []
    @cache.servers << server

    e = assert_raise Memcached::MemcachedError do
      @cache.get 'test:key'
    end

    assert_equal 'some io error', e.message
  end

  def test_get_cache_get_SystemCallError
    socket = Object.new
    def socket.write(arg) raise SystemCallError, 'some syscall error'; end
    server = FakeServer.new socket

    @cache.servers = []
    @cache.servers << server

    e = assert_raise Memcached::MemcachedError do
      @cache.get 'test:key'
    end

    assert_equal 'unknown error - some syscall error', e.message
  end

  def test_get_no_connection
    @cache.servers = 'localhost:1'
    e = assert_raise Memcached::MemcachedError do
      @cache.get 'key'
    end

    assert_equal 'No connection to server', e.message
  end

  def test_get_no_servers
    @cache.servers = []
    e = assert_raise Memcached::MemcachedError do
      @cache.get 'key'
    end

    assert_equal 'No active servers', e.message
  end

  def test_get_multi
    server = FakeServer.new
    server.socket.data.write "VALUE test:key 0 14\r\n"
    server.socket.data.write "\004\b\"\0170123456789\r\n"
    server.socket.data.write "VALUE test:keyb 0 14\r\n"
    server.socket.data.write "\004\b\"\0179876543210\r\n"
    server.socket.data.write "END\r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server

    values = @cache.get_multi 'key', 'keyb'

    assert_equal "get test:key test:keyb\r\n",
                 server.socket.written.string

    expected = { 'key' => '0123456789', 'keyb' => '9876543210' }

    assert_equal expected.sort, values.sort
  end

  def test_get_raw
    server = FakeServer.new
    server.socket.data.write "VALUE test:key 0 10\r\n"
    server.socket.data.write "0123456789\r\n"
    server.socket.data.write "END\r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server


    value = @cache.get 'key', true

    assert_equal "get test:key\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal '0123456789', value
  end

  def test_get_server_for_key
    server = @cache.get_server_for_key 'key'
    assert_equal 'localhost', server.host
    assert_equal 1, server.port
  end

  def test_get_server_for_key_multiple
    s1 = util_setup_server @cache, 'one.example.com', ''
    s2 = util_setup_server @cache, 'two.example.com', ''
    @cache.instance_variable_set :@servers, [s1, s2]
    @cache.instance_variable_set :@buckets, [s1, s2]

    server = @cache.get_server_for_key 'keya'
    assert_equal 'two.example.com', server.host
    server = @cache.get_server_for_key 'keyb'
    assert_equal 'one.example.com', server.host
  end

  def test_get_server_for_key_no_servers
    @cache.servers = []

    e = assert_raise Memcached::MemcachedError do
      @cache.get_server_for_key 'key'
    end

    assert_equal 'No servers available', e.message
  end

  def test_get_server_for_key_spaces
    e = assert_raise ArgumentError do
      @cache.get_server_for_key 'space key'
    end
    assert_equal 'illegal character in key "space key"', e.message
  end

  def test_get_server_for_key_length
    @cache.get_server_for_key 'x' * 250
    long_key = 'x' * 251
    e = assert_raise ArgumentError do
      @cache.get_server_for_key long_key
    end
    assert_equal "key too long #{long_key.inspect}", e.message
  end

  def test_incr
    server = FakeServer.new
    server.socket.data.write "5\r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server

    value = @cache.incr 'key'

    assert_equal "incr test:key 1\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal 5, value
  end

  def test_incr_not_found
    server = FakeServer.new
    server.socket.data.write "NOT_FOUND\r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server

    value = @cache.incr 'key'

    assert_equal "incr test:key 1\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal nil, value
  end

  def test_incr_space_padding
    server = FakeServer.new
    server.socket.data.write "5 \r\n"
    server.socket.data.rewind

    @cache.servers = []
    @cache.servers << server

    value = @cache.incr 'key'

    assert_equal "incr test:key 1\r\n",
                 @cache.servers.first.socket.written.string

    assert_equal 5, value
  end

  def test_make_cache_key
    assert_equal 'test:key', @cache.make_cache_key('key')
    @cache.namespace = nil
    assert_equal 'key', @cache.make_cache_key('key')
  end

  def test_servers
    server = FakeServer.new
    @cache.servers = []
    @cache.servers << server
    assert_equal [server], @cache.servers
  end

  def test_servers_equals_type_error
    e = assert_raise TypeError do
      @cache.servers = [Object.new]
    end

    assert_equal 'cannot convert Object into Memcached::Server', e.message
  end

  def test_set
    server = FakeServer.new
    server.socket.data.write "STORED\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    @cache.set 'key', 'value'

    expected = "set test:key 0 0 9\r\n\004\b\"\nvalue\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_set_expiry
    server = FakeServer.new
    server.socket.data.write "STORED\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    @cache.set 'key', 'value', 5

    expected = "set test:key 0 5 9\r\n\004\b\"\nvalue\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_set_raw
    server = FakeServer.new
    server.socket.data.write "STORED\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    @cache.set 'key', 'value', 0, true

    expected = "set test:key 0 0 5\r\nvalue\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_set_too_big
    server = FakeServer.new
    server.socket.data.write "SERVER_ERROR object too large for cache\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    e = assert_raise Memcached::MemcachedError do
      @cache.set 'key', 'v'
    end

    assert_equal 'object too large for cache', e.message
  end

  def test_add
    server = FakeServer.new
    server.socket.data.write "STORED\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    @cache.add 'key', 'value'

    expected = "add test:key 0 0 9\r\n\004\b\"\nvalue\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_add_exists
    server = FakeServer.new
    server.socket.data.write "NOT_STORED\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    @cache.add 'key', 'value'

    expected = "add test:key 0 0 9\r\n\004\b\"\nvalue\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_add_expiry
    server = FakeServer.new
    server.socket.data.write "STORED\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    @cache.add 'key', 'value', 5

    expected = "add test:key 0 5 9\r\n\004\b\"\nvalue\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_add_raw
    server = FakeServer.new
    server.socket.data.write "STORED\r\n"
    server.socket.data.rewind
    @cache.servers = []
    @cache.servers << server

    @cache.add 'key', 'value', 0, true

    expected = "add test:key 0 0 5\r\nvalue\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_delete
    server = FakeServer.new
    @cache.servers = []
    @cache.servers << server
    
    @cache.delete 'key'
    
    expected = "delete test:key 0\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_delete_with_expiry
    server = FakeServer.new
    @cache.servers = []
    @cache.servers << server
    
    @cache.delete 'key', 300
    
    expected = "delete test:key 300\r\n"
    assert_equal expected, server.socket.written.string
  end

  def test_flush_all
    @cache.servers = []
    3.times { @cache.servers << FakeServer.new }

    @cache.flush_all

    expected = "flush_all\r\n"
    @cache.servers.each do |server|
      assert_equal expected, server.socket.written.string
    end
  end

  def test_flush_all_failure
    socket = FakeSocket.new
    socket.data.write "ERROR\r\n"
    socket.data.rewind
    server = FakeServer.new socket
    def server.host() "localhost"; end
    def server.port() 11211; end

    @cache.servers = []
    @cache.servers << server

    assert_raise Memcached::MemcachedError do
      @cache.flush_all
    end

    assert_equal "flush_all\r\n", socket.written.string
  end

  def test_stats
    socket = FakeSocket.new
    socket.data.write "STAT pid 20188\r\nSTAT total_items 32\r\nSTAT version 1.2.3\r\nSTAT rusage_user 1:300\r\nSTAT dummy ok\r\nEND\r\n"
    socket.data.rewind
    server = FakeServer.new socket
    def server.host() 'localhost'; end
    def server.port() 11211; end

    @cache.servers = []
    @cache.servers << server

    expected = {
      'localhost:11211' => {
        'pid' => 20188, 'total_items' => 32, 'version' => '1.2.3',
        'rusage_user' => 1.0003, 'dummy' => 'ok'
      }
    }
    assert_equal expected, @cache.stats

    assert_equal "stats\r\n", socket.written.string
  end

  def test_basic_threaded_operations_should_work
    cache = Memcached.new :multithread => true,
                         :namespace => 'test'
    server = util_setup_server cache, 'example.com', "OK\r\n"
    cache.instance_variable_set :@servers, [server]

    assert_nothing_raised do
      cache.set "test", "test value"
    end
  end

  def test_basic_unthreaded_operations_should_work
    cache = Memcached.new :multithread => false,
                         :namespace => 'test'
    server = util_setup_server cache, 'example.com', "OK\r\n"
    cache.instance_variable_set :@servers, [server]

    assert_nothing_raised do
      cache.set "test", "test value"
    end
  end

end

