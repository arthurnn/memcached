require 'taj'

module Taj
  class Connection
    def self.create(_)
      self.new
    end
  end
end

module Memcached
  class Client

    def initialize(servers = nil)
      @servers = servers
    end

    def set(key, value, ttl: @default_ttl, encode: true)#, flags: FLAGS)
      connection.set(key, value.to_s)
    end

    def connection
      @connection ||= Taj::Connection.create(@servers)
    end
  end
#  raise "libmemcached 0.32 required; you somehow linked to #{Lib.memcached_lib_version}." unless "0.32" == Lib.memcached_lib_version
#  VERSION = File.read("#{File.dirname(__FILE__)}/../CHANGELOG")[/v([\d\.]+)\./, 1]
end

#require 'memcached/exceptions'
#require 'memcached/behaviors'
#require 'memcached/auth'
#require 'memcached/marshal_codec'
#require 'memcached/memcached'
#require 'memcached/rails'
#require 'memcached/experimental'
