require 'memcached/marshal_codec'

module Memcached

  module Behaviors
    BASE_VALUES = {
      MEMCACHED_BEHAVIOR_HASH => "MEMCACHED_HASH_%s",
      MEMCACHED_BEHAVIOR_DISTRIBUTION => "MEMCACHED_DISTRIBUTION_%s",
    }

    def lookup_value(behavior, value)
      base_value = BASE_VALUES[behavior]
      return false unless base_value
      value = base_value % value.to_s.upcase
      Behaviors.const_get(value)
#    rescue NameError => e
#      return false #  TODO
    end

    def set_behaviors(hash)
      hash.each do |key, value|
        behavior = "MEMCACHED_BEHAVIOR_#{key.to_s.upcase}"
        behavior = Behaviors.const_get(behavior) # TODO NameError
        if value.is_a? Symbol
          value = lookup_value(behavior, value)
        end
        set_behavior(behavior, value)
      end

    end
  end

  class Connection
    include Behaviors
  end

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
      @behaviors = normalize_behaviors({hash: :fnv1_32, noreply: false, distribution: :consistent_ketama}) # TODO
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

    def prepend(key, value, ttl: @default_ttl, flags: FLAGS)
      connection.prepend(key, value, ttl, flags)
    end

    def append(key, value, ttl: @default_ttl, flags: FLAGS)
      connection.append(key, value, ttl, flags)
    end

    def connection
      @connection ||= Memcached::Connection.new(@servers).tap do |conn|
        conn.set_behaviors(@behaviors)
      end
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

    # TODO
    def normalize_behaviors(options)
      options
    end

  end
end
