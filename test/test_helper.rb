require 'memcached'

require 'ostruct'
require 'securerandom'

require 'minitest/autorun'
require 'mocha/mini_test'

class BaseTest < Minitest::Test
  UNIX_SOCKET_NAME = File.join('/tmp', 'memcached')

  def setup
    @servers = ['localhost:43042', 'localhost:43043', "#{UNIX_SOCKET_NAME}0"]
    @udp_servers = ['localhost:43052', 'localhost:43053']

    @value = OpenStruct.new(a: 1, b: 2, c: self.class)
    @marshalled_value = Marshal.dump(@value)
    @cache = nil
    @binary_protocol_cache = nil
  end

  def teardown
    @cache.flush if @cache
    @binary_protocol_cache.flush if @binary_protocol_cache
  end

  private

  def cache
    return @cache if @cache
    @cache = Memcached::Client.new(@servers) #, @options)
  end

  def binary_protocol_cache
    return @binary_protocol_cache if @binary_protocol_cache
    binary_protocol_options = {
#      :prefix_key => @prefix_key,
      :hash => :default,
      :distribution => :modula,
      :binary_protocol => true
    }
    @binary_protocol_cache = Memcached::Client.new(@servers, binary_protocol_options)
  end

  def key
    caller.first[/.*[` ](.*)'/, 1]
  end
end
