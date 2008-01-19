
require "#{File.dirname(__FILE__)}/../test_helper"

begin
  require 'ruby-debug'
rescue LoadError
end

class ClassTest < Test::Unit::TestCase

  def setup
    @cache = Memcached.new(
      ['localhost:43042', '127.0.0.1:43043'], 
      :namespace => 'test'
    )
  end

  def test_initialize
    cache = Memcached.new ['localhost:43042', '127.0.0.1:43043'], :namespace => 'test'
    assert_equal 'test', cache.namespace
    assert_equal ['localhost:43042', '127.0.0.1:43043'], cache.servers
  end

  def test_initialize_without_hash
    cache = Memcached.new ['localhost:43042', '127.0.0.1:43043']
    assert_equal nil, cache.namespace
    assert_equal ['localhost:43042', '127.0.0.1:43043'], cache.servers
  end

  def test_initialize_single_server
    cache = Memcached.new 'localhost:43042'
    assert_equal nil, cache.namespace
    assert_equal ['localhost:43042'], cache.servers
  end

  def test_initialize_bad_argument
    assert_raise(ArgumentError) { Memcached.new 1 }
  end

#  def test_get
#    value = @cache.get 'get'
#    assert_equal '0123456789', value
#  end
#
#  def test_get_bad_key
#    assert_raise ArgumentError { @cache.get "I'm so bad" }
#    assert_raise ArgumentError { @cache.get('key' * 100) }
#  end
#
#  def test_get_no_connection
#    @cache.servers = 'localhost:1'
#    assert_raise Memcached::ConnectionError do
#      @cache.get 'key'
#    end
#  end
#
#  def test_get_multi
#    values = @cache.get_multi 'get-one', 'get-two'
#  end
#
#  def test_get_raw
#    value = @cache.get 'key', true
#    assert_equal '0123456789', value
#  end
#
#  def test_incr
#    value = @cache.incr 'incr'
#    assert_equal 5, value
#  end
#
#  def test_incr_not_found
#  end
#
#  def test_decr
#    value = @cache.decr 'decr'
#  end
#
#  def test_decr_not_found
#  end
#
#  def test_set
#    @cache.set 'key', 'value'
#  end
#
#  def test_set_expiry
#  end
#
#  def test_set_raw
#  end
#
#  def test_set_object_too_large
#  end
#
#  def test_add
#    @cache.add 'definitely-not-there-key', 'value'
#  end
#
#  def test_add_exists
#    @cache.add 'already-there-key', 'value'
#  end
#
#  def test_add_expiry
#    @cache.add 'key', 'value', 5
#  end
#
#  def test_add_raw
#    @cache.add 'key', 'value', 0, true
#  end
#
#  def test_delete
#    @cache.delete 'key'
#  end
#
#  def test_delete_with_expiry
#    @cache.delete 'key', 300
#  end
#
#  def test_stats
#  end
#
#  def test_thread_contention
#  end

end

