require File.expand_path("#{File.dirname(__FILE__)}/../test_helper")
require 'active_support/duration'
require 'logger'

class RailsTest < Test::Unit::TestCase

  def setup
    @servers = ['127.0.0.1:43042', '127.0.0.1:43043', "#{UNIX_SOCKET_NAME}0"]
    @duration = ActiveSupport::Duration.new(2592000, [[:months, 1]])
    @namespace = 'rails_test'
    @cache = Memcached::Rails.new(:servers => @servers, :namespace => @namespace, :logger => Logger.new(File.open("/dev/null", "w")))
    @value = OpenStruct.new(:a => 1, :b => 2, :c => GenericClass)
    @marshalled_value = Marshal.dump(@value)
  end

  def test_get
    @cache.set key, @value
    result = @cache.get key
    assert_equal @value, result
  end

  def test_exist
    @cache.delete(key)
    assert !@cache.exist?(key)
    @cache.set key, nil
    assert @cache.exist?(key)
    @cache.set key, @value
    assert @cache.exist?(key)
  end

  def test_get_multi
    @cache.set key, @value
    assert_equal(
      {key => @value},
      @cache.get_multi([key]))
  end

  def test_get_multi_empty_string
    @cache.set key, "", 0, true
    assert_equal(
      {key => ""},
      @cache.get_multi([key], true))
  end

  def test_delete
    @cache.set key, @value
    assert_nothing_raised do
      @cache.delete key
    end
    assert_nil(@cache.get(key))
  end

  def test_delete_missing
    assert_nothing_raised do
      @cache.delete key
      assert !@cache.delete(key)
    end
  end

  def test_delete_with_two_arguments
    assert_nothing_raised do
      @cache.delete(key, 5)
      assert_nil(@cache.get(key))
    end
  end

  def test_bracket_accessors
    @cache[key] = @value
    result = @cache[key]
    assert_equal @value, result
  end

  def test_cas
    cache = Memcached::Rails.new(:servers => @servers, :namespace => @namespace, :support_cas => true)
    value2 = OpenStruct.new(:d => 3, :e => 4, :f => GenericClass)

    # Existing set
    cache.set key, @value
    cache.cas(key) do |current|
      assert_equal @value, current
      value2
    end
    assert_equal value2, cache.get(key)

    # Missing set
    cache.delete key
    assert_nothing_raised do
      cache.cas(key) { @called = true }
    end
    assert_nil cache.get(key)
    assert_nil @called

    # Conflicting set after a gets
    cache.set key, @value
    assert_nothing_raised do
      result = cache.cas(key) do |current|
        cache.set key, value2
        current
      end
      assert_equal result, false
    end
    assert_equal value2, cache.get(key)
  end

  def test_get_missing
    @cache.delete key rescue nil
    result = @cache.get key
    assert_equal nil, result
  end

  def test_get_nil
    @cache.set key, nil, 0
    result = @cache.get key
    assert_equal nil, result
  end

  def test_namespace
    assert_equal @namespace, @cache.namespace
  end

  def test_active
    assert @cache.active?
  end

  def test_servers
    compare_servers @cache, @servers
  end

  def test_servers_alive
    @cache.servers.each do |server|
      assert server.alive?
    end
  end

  def test_set_servers
    cache = Memcached::Rails.new(:servers => @servers, :namespace => @namespace)
    compare_servers cache, @servers
    cache.set_servers cache.servers
    cache = Memcached::Rails.new(:servers => @servers.first, :namespace => @namespace)
    cache.set_servers @servers.first
  end

  def test_cas_with_duration
    cache = Memcached::Rails.new(:servers => @servers, :namespace => @namespace, :support_cas => true)
    value2 = OpenStruct.new(:d => 3, :e => 4, :f => GenericClass)
    cache.set key, @value
    cache.cas(key, @duration) do |current|
      assert_equal @value, current
      value2
    end
    assert_equal value2, cache.get(key)
  end

  def test_set_with_duration
    @cache.set key, @value, @duration
    result = @cache.get key
    assert_equal @value, result
  end

  def test_set_with_invalid_type
    assert_raises(TypeError) do
      @cache.set key, Object.new, 0, true
    end
  end

  def test_write_with_default_duration
    # should be do an actual write
    @cache.write key, @value
    result = @cache.get key
    assert_equal @value, result

    # should use correct ttl
    @cache = Memcached::Rails.new(:servers => @servers, :namespace => @namespace, :default_ttl => 123)
    @cache.expects(:set).with(key, @value, 123, nil)
    @cache.write key, @value
  end

  def test_write_with_duration_as_ttl
    # should be do an actual write
    @cache.write key, @value, :ttl => 123
    result = @cache.get key
    assert_equal @value, result

    # should use correct ttl
    @cache.expects(:set).with(key, @value, 123, nil)
    @cache.write key, @value, :ttl => 123
  end

  def test_write_with_duration_as_expires_in
    # should be do an actual write
    @cache.write key, @value, :expires_in => @duration
    result = @cache.get key
    assert_equal @value, result

    # should use correct ttl
    @cache.expects(:set).with(key, @value, 123, nil)
    @cache.write key, @value, :expires_in => 123
  end

  def test_write_with_unless_exist
    @cache.write key, @value, :unless_exist => true
    result = @cache.get key
    assert_equal @value, result

    rand_value = "rand-value-#{rand}"
    @cache.write key, rand_value, :unless_exist => true

    result = @cache.get key
    assert_equal @value, result
  end

  def test_write_with_fixnum_and_raw
    @cache.write key, 1, :raw => true
    result = @cache.read key, :raw => true
    assert_equal "1", result
  end

  def test_add_with_duration
    @cache.add key, @value, @duration
    result = @cache.get key
    assert_equal @value, result
  end

  def test_fetch
    # returns
    assert_equal @value, @cache.fetch("x"){ @value }

    # sets
    assert_equal @value, @cache.read("x")

    # reads
    assert_equal @value, @cache.fetch("x"){ 1 }

    # works with options
    @cache.expects(:write).with("y", 1, :foo => :bar)
    @cache.fetch("y", :foo => :bar){ 1 }
  end

  def test_fetch_when_no_block_given
    rand_key = "key-#{rand}"

    # gets nil the first time
    assert_equal nil, @cache.fetch(rand_key)

    # sets value to @value
    assert_equal @value, @cache.fetch(rand_key) { @value }

    # gets @value
    assert_equal @value, @cache.fetch(rand_key)
  end

  def test_read_multi
    # empty
    assert_equal({}, @cache.read_multi)
    assert_equal({}, @cache.read_multi("a", "b"))
    assert_raise TypeError do
      @cache.read_multi(["a", "b"])
    end

    # filled
    @cache.write "a", @value
    assert_equal({"a" => @value}, @cache.read_multi("a", "b"))
    assert_raise TypeError do
      @cache.read_multi(["a", "b"])
    end

    # multiple keys
    @cache.write "b", 1
    assert_equal({"a" => @value, "b" => 1}, @cache.read_multi("a", "b"))

    # keeps order on 1.9
    if RUBY_VERSION =~ /^1.9/
      assert_equal ["a", "b"], @cache.read_multi("a", "b").keys
      assert_equal ["b", "a"], @cache.read_multi("b", "a").keys
    end
  end

  def test_clear
    @cache.write("x", 1)
    assert_equal 1, @cache.read("x")
    assert_equal nil, @cache.clear
    assert_equal nil, @cache.read("x")
  end

  def test_increment
    rand_key = "rand-key-#{rand}"
    assert_equal nil, @cache.increment(rand_key)

    start = 10
    @cache.write rand_key, start.to_s, { :raw => true }

    assert_equal start, @cache.read(rand_key, { :raw => true }).to_i
    assert_equal start+1, @cache.increment(rand_key)

    assert_equal start+1+5, @cache.increment(rand_key, 5)
  end

  def test_decrement
    rand_key = "rand-key-#{rand}"
    assert_equal nil, @cache.decrement(rand_key)

    start = 10
    @cache.write rand_key, start.to_s, { :raw => true }

    assert_equal start, @cache.read(rand_key, { :raw => true }).to_i
    assert_equal start-1, @cache.decrement(rand_key)

    assert_equal start-1-5, @cache.decrement(rand_key, 5)
  end

  def test_read_with_nil_options
    @cache.write key, @value
    assert_equal @value, @cache.read(key, nil)
  end

  def test_write_with_nil_options
    @cache.write key, @value, nil
    assert_equal @value, @cache.read(key)
  end

  private

  def key
    caller.first[/.*[` ](.*)'/, 1] # '
  end

  def compare_servers(cache, servers)
    cache.servers.map{|s| "#{s.hostname}:#{s.port}".chomp(':0')} == servers
  end
end
