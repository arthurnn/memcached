require 'test_helper'

class BehaviorTest < BaseTest
  def test_setting_value_string
    assert_equal 0, cache.connection.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH)
    cache.connection.set_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH, "MEMCACHED_HASH_JENKINS")
    assert_equal Memcached::Behaviors::MEMCACHED_HASH_JENKINS, cache.connection.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH)
  end

  def test_setting_value_with_constant
    assert_equal 0, cache.connection.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH)
    cache.connection.set_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH, Memcached::Behaviors::MEMCACHED_HASH_FNV1_64)
    assert_equal Memcached::Behaviors::MEMCACHED_HASH_FNV1_64, cache.connection.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH)
  end

  def test_setting_value_with_bool
    cache.connection.set_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_USE_UDP, true)
    assert cache.connection.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_USE_UDP)
  end

  def test_setting_value_with_invalid_num
    cache.connection.set_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH, 100)
    assert_equal 0, cache.connection.get_behavior(Memcached::Behaviors::MEMCACHED_BEHAVIOR_HASH)
  end
end
