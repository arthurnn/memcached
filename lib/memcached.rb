require 'taj'

require 'memcached/marshal_codec'

module Memcached
  class Client
    FLAGS = 0x0

    attr_reader :servers

    def initialize(servers = nil)
      if servers
        @servers = normalize_servers(servers)
      else
        @servers = [[:tcp, "localhost", 11211]]
      end

      @codec = Memcached::MarshalCodec
    end

    def flush
      connection.flush
    end

    def set(key, value, ttl: 0, encode: true, flags: FLAGS)
      value, flags = @codec.encode(key, value, flags) if encode

      connection.set(key, value)
    end

    def get(key)
      value = connection.get(key)
      return nil unless value
      value = @codec.decode(key, value, FLAGS)# if decode
      value
    end

    def get_multi(keys)
      hash = connection.get_multi(keys)
      hash.each do |key, value|
        hash[key] = @codec.decode(key, value, FLAGS)
      end
    end

    def connection
      @connection ||= Taj::Connection.new(@servers)
    end

    private
    def normalize_servers(servers)
      servers.map do |server|
        server = server.to_s
        if server =~ /^[\w\d\.-]+(:\d{1,5}){0,2}$/
          host, port, _weight = server.split(":")
          # TODO weight
          [:tcp, host, port.to_i]
        elsif File.socket?(server)
          # TODO weight, default = 8
          [:socket, server]
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
#require 'memcached/memcached'
#require 'memcached/rails'
#require 'memcached/experimental'
