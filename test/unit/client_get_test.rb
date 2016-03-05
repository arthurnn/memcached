require 'test_helper'

require 'ostruct'
require 'securerandom'

class ClientGetTest < Minitest::Test

  def setup
    @cache = Memcached::Client.new#(@servers, @options)
    @cache.flush

    @value = OpenStruct.new(a: 1, b: 2, c: self.class)
    @marshalled_value = Marshal.dump(@value)
  end

  def key
    caller.first[/.*[` ](.*)'/, 1]
  end

  def test_simple_get
    @cache.set "foo", "bar"
    assert_equal "bar", @cache.get("foo")

    @cache.set key, @value
    assert_equal @value, @cache.get(key)
  end

#  def test_binary_get
#    @cache.set key, @value
#    assert_equal @value, @binary_protocol_cache.get(key)
#  end
#
#  def test_udp_get
#    @udp_cache.set(key, @value)
#    assert_raises(Memcached::ActionNotSupported) do
#      @udp_cache.get(key)
#    end
#  end

  def test_get_nil
    @cache.set key, nil, ttl:  0
    result = @cache.get key
    assert_equal nil, result
  end

#  def test_get_from_last
#    cache = Memcached.new(@servers, :distribution => :random)
#    10.times { |n| cache.set key, n }
#    10.times do
#      assert_equal cache.get(key), cache.get_from_last(key)
#    end
#  end
#
  def test_get_missing
    assert_nil @cache.get "#{key}/#{SecureRandom.hex}"
  end

  def test_get_coerces_string_type
    assert_raises(TypeError) do
      @cache.get nil
    end

    assert_raises(TypeError) do
      @cache.get 1
    end
  end

#  def test_get_with_server_timeout
#    socket = stub_server 43047
#    cache = Memcached.new("localhost:43047:1", :timeout => 0.5, :exception_retry_limit => 0)
#    assert 0.49 < (Benchmark.measure do
#      assert_raise(Memcached::ATimeoutOccurred) do
#        result = cache.get key
#      end
#    end).real
#
#    cache = Memcached.new("localhost:43047:1", :poll_timeout => 0.001, :rcv_timeout => 0.5, :exception_retry_limit => 0)
#    assert 0.49 < (Benchmark.measure do
#      assert_raise(Memcached::ATimeoutOccurred) do
#        result = cache.get key
#      end
#    end).real
#
#    cache = Memcached.new("localhost:43047:1", :poll_timeout => 0.25, :rcv_timeout => 0.25, :exception_retry_limit => 0)
#    assert 0.51 > (Benchmark.measure do
#      assert_raise(Memcached::ATimeoutOccurred) do
#        result = cache.get key
#      end
#    end).real
#  ensure
#    socket.close
#  end
#
#  def test_get_with_no_block_server_timeout
#    socket = stub_server 43048
#    cache = Memcached.new("localhost:43048:1", :no_block => true, :timeout => 0.25, :exception_retry_limit => 0)
#    assert 0.24 < (Benchmark.measure do
#      assert_raise(Memcached::ATimeoutOccurred) do
#        result = cache.get key
#      end
#    end).real
#
#    cache = Memcached.new("localhost:43048:1", :no_block => true, :poll_timeout => 0.25, :rcv_timeout => 0.001, :exception_retry_limit => 0)
#    assert 0.24 < (Benchmark.measure do
#      assert_raise(Memcached::ATimeoutOccurred) do
#        result = cache.get key
#      end
#    end).real
#
#    cache = Memcached.new("localhost:43048:1", :no_block => true,
#      :poll_timeout => 0.001,
#      :rcv_timeout => 0.25, # No affect in no-block mode
#      :exception_retry_limit => 0
#    )
#    assert 0.24 > (Benchmark.measure do
#      assert_raise(Memcached::ATimeoutOccurred) do
#        result = cache.get key
#      end
#    end).real
#
#  ensure
#    socket.close
#  end
#
#  def test_get_with_prefix_key
#    # Prefix_key
#    cache = Memcached.new(
#      # We can only use one server because the key is hashed separately from the prefix key
#      @servers.first,
#      :prefix_key => @prefix_key,
#      :hash => :default,
#      :distribution => :modula
#    )
#    cache.set key, @value
#    assert_equal @value, cache.get(key)
#
#    # No prefix_key specified
#    cache = Memcached.new(
#      @servers.first,
#      :hash => :default,
#      :distribution => :modula
#    )
#    assert_nothing_raised do
#      assert_equal @value, cache.get("#{@prefix_key}#{key}")
#    end
#  end
#
#  def test_values_with_null_characters_are_not_truncated
#    value = OpenStruct.new(:a => Object.new) # Marshals with a null \000
#    @cache.set key, value
#    result = @cache.get key, false
#    non_wrapped_result = Rlibmemcached.memcached_get(
#      @cache.instance_variable_get("@struct"),
#      key
#    ).first
#    assert result.size > non_wrapped_result.size
#  end
#
  def test_get_multi
    @cache.set "#{key}_1", 1
    @cache.set "#{key}_2", 2
    assert_equal({"#{key}_1" => 1, "#{key}_2" => 2},
                 @cache.get_multi(["#{key}_1", "#{key}_2"]))
  end

  def test_get_multi_missing
    keys = 4.times.map { |n| "#{key}/#{n}" }
    @cache.set keys[0], 1
    @cache.set keys[1], 3
    assert_equal({keys[0] => 1, keys[1] => 3}, @cache.get_multi(keys))
  end
#
#  def test_get_multi_binary
#    @binary_protocol_cache.set "#{key}_1", 1
#    @binary_protocol_cache.delete "#{key}_2" rescue nil
#    @binary_protocol_cache.set "#{key}_3", 3
#    assert_equal(
#      {"test_get_multi_binary_3"=>3, "test_get_multi_binary_1"=>1},
#      @binary_protocol_cache.get(["#{key}_1", "#{key}_2",  "#{key}_3"])
#     )
#  end
#
#  def test_get_multi_binary_one_record_missing
#    @binary_protocol_cache.delete("magic_key") rescue nil
#    assert_equal({}, @binary_protocol_cache.get(["magic_key"]))
#  end
#
#  def test_get_multi_binary_one_record
#    @binary_protocol_cache.set("magic_key", 1)
#    assert_equal({"magic_key" => 1}, @binary_protocol_cache.get(["magic_key"]))
#  end
#
  def test_get_multi_completely_missing
    assert_equal({}, @cache.get_multi(["#{key}_1", "#{key}_2"]))
  end

  def test_get_multi_empty_string
    @cache.set "#{key}_1", "", ttl: 0, raw: true
    assert_equal({"#{key}_1" => ""},
                 @cache.get_multi(["#{key}_1"], raw: true))
  end

  def test_get_multi_coerces_string_type
    assert @cache.get_multi [nil]

    assert_raises(TypeError) do
      @cache.get_multi [1]
    end
  end

  def test_set_and_get_unmarshalled
    @cache.set key, @value
    result = @cache.get key, raw: true
    assert_equal @marshalled_value, result
  end

  def test_set_unmarshalled_and_get_unmarshalled
    @cache.set key, @marshalled_value, raw: true
    result = @cache.get key, raw: true
    assert_equal @marshalled_value, result
  end

  def test_set_unmarshalled_error
    assert_raises(TypeError) do
      @cache.set key, @value, raw: true
    end
  end

  def test_get_multi_unmarshalled
    @cache.set "#{key}_1", "1", raw: true
    @cache.set "#{key}_2", "2", raw: true
    assert_equal(
      {"#{key}_1" => "1", "#{key}_2" => "2"},
      @cache.get_multi(["#{key}_1", "#{key}_2"], raw: true)
    )
  end

  def test_get_multi_mixed_marshalling
    @cache.set "#{key}_1", 1
    @cache.set "#{key}_2", "2", raw: true

    assert @cache.get_multi(["#{key}_1", "#{key}_2"], raw: true)

    assert_raises(ArgumentError, TypeError) do
      @cache.get_multi(["#{key}_1", "#{key}_2"])
    end
  end

#  def test_random_distribution_is_statistically_random
#    cache = Memcached.new(@servers, :distribution => :random)
#    read_cache = Memcached.new(@servers.first)
#    hits = 4
#
#    while hits == 4 do
#      cache.flush
#      20.times do |i|
#        cache.set "#{key}#{i}", @value
#      end
#
#      hits = 0
#      20.times do |i|
#        begin
#          read_cache.get "#{key}#{i}"
#          hits += 1
#        rescue Memcached::NotFound
#        end
#      end
#    end
#
#    assert_not_equal 4, hits
#  end
end
