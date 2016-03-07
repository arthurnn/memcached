require 'memcached/marshal_codec'

module Memcached
  class Client
    FLAGS = 0x0

    attr_reader :servers

    def initialize(servers = nil, ttl: 0)
      if servers
        @servers = normalize_servers(servers)
      else
        @servers = [[:tcp, "localhost", 11211]]
      end

      @codec = Memcached::MarshalCodec
      @default_ttl = ttl
    end

    def flush
      connection.flush
    end

    def set(key, value, ttl: @default_ttl, raw: false, flags: FLAGS)
      return false unless key

      value, flags = @codec.encode(key, value, flags) unless raw
      connection.set(key, value, ttl, flags)
    end

    def get(key, raw: false)
      value = connection.get(key)
      return nil unless value
      value = @codec.decode(key, value, FLAGS) unless raw
      value
    end

    def get_multi(keys, raw: false)
      keys = keys.compact
      hash = connection.get_multi(keys)
      unless raw
        hash.each do |key, value|
          hash[key] = @codec.decode(key, value, FLAGS)
        end
      end
      hash
    end

    def delete(key)
      connection.delete(key)
    end

    def add(key, value, ttl: @default_ttl, raw: false, flags: FLAGS)
      return false unless key

      value, flags = @codec.encode(key, value, flags) unless raw
      connection.add(key, value, ttl, flags)
    end

    def increment(key, offset = 1)
      connection.increment(key, offset)
    end

    def decrement(key, offset = 1)
      connection.decrement(key, offset)
    end

    def exist(key)
      connection.exist(key)
    end

    def replace(key, value, ttl: @default_ttl, raw: false, flags: FLAGS)
      return false unless key

      value, flags = @codec.encode(key, value, flags) unless raw
      connection.replace(key, value, ttl, flags)
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
end
