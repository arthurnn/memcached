#require 'test_helper'
require 'memcached'

require 'test/unit'
require 'test/unit/assertions'
require 'mocha/test_unit'

class ClientTest < Test::Unit::TestCase

  def setup
    @servers = ['localhost:43042', 'localhost:43043']
  end

  def test_initialize_without_servers
    client = Memcached::Client.new
    assert_equal [["localhost", 11211]], client.connection.servers
  end

  def test_initialize_with_multiple_servers
    client = Memcached::Client.new @servers
    assert_equal [["localhost", 43042], ["localhost", 43043]], client.connection.servers
  end

  def test_set
    client = Memcached::Client.new
    assert client.set('foo', 'baz')
    assert client.set('foo', true)
  end
end
