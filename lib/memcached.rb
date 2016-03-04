require 'taj'

module Memcached
  class Client

    def initialize(servers = nil)
      if servers
        @servers = normalize_servers(servers)
      else
        @servers = [["localhost", 11211]]
      end
    end

    def set(key, value, ttl: @default_ttl, encode: true)#, flags: FLAGS)
      connection.set(key, value.to_s)
    end

    def connection
      @connection ||= Taj::Connection.new(@servers)
    end

    private
    def normalize_servers(servers)
      servers.map do |server|
        if server.is_a?(String) && server =~ /^[\w\d\.-]+(:\d{1,5}){0,2}$/
          host, port, weight = server.split(":")
          [host, port.to_i, weight]
        else
          nil
        end
      end.compact
    rescue
      raise ArgumentError, <<-MSG
      Servers must be either in the format 'host:port[:weight]' (e.g., 'localhost:11211' or 'localhost:11211:10') for a network server, or a valid path to a Unix domain socket (e.g., /var/run/memcached).
But it was #{servers.inspect}.
          MSG
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
