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
  end

  private

  def cache
    return @cache if @cache
    @cache = Memcached::Client.new(@servers) #, @options)
    @cache.flush
    @cache
  end

  def key
    caller.first[/.*[` ](.*)'/, 1]
  end
end
