require 'memcached/marshal_codec'
require 'memcached/behaviors'

module Memcached
  class Connection
    include Behaviors
  end

  class Client
    FLAGS = 0x0

    DEFAULTS = {
      :hash => :fnv1_32,
      :distribution => :consistent_ketama,
      :ketama_weighted => true,
      :retry_timeout => 60,
      :timeout => 0.25,
#      :poll_max_retries => 1, TODO: doesnt exist anymore
      :connect_timeout => 0.25,
      :hash_with_prefix_key => true,
      :default_weight => 8,
      :server_failure_limit => 2,
      :verify_key => true,
    }

    attr_reader :servers, :options

    def initialize(servers = nil, options = {})
      if servers
        @servers = normalize_servers(servers)
      else
        @servers = [[:tcp, "localhost", 11211]]
      end
      @options = DEFAULTS.merge(options)

      @codec = Memcached::MarshalCodec
      @default_ttl = @options.delete(:ttl) || 0
      prefix = @options.delete(:prefix_key)

      @default_weight = @options.delete(:default_weight) # TODO
      @options.delete(:credentials) # TODO

      @behaviors = normalize_behaviors(@options)

      self.namespace = prefix if prefix

      @options.freeze
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

    def namespace
      connection.get_prefix
    end

    def namespace=(value)
      connection.set_prefix(value)
    end

    def touch(key, ttl = @default_ttl)
      connection.touch(key, ttl)
    end

    def clone
      client = super
      client.instance_variable_set(:@connection, @connection.clone) if @connection
      client
    end

    def connection
      @connection ||= Memcached::Connection.new(@servers).tap do |conn|
        conn.set_behaviors(@behaviors)
      end
    end

    private
    def normalize_servers(servers)
      servers = [servers] if servers.is_a?(String)
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
          raise
        end
      end
    rescue
      raise ArgumentError, <<-MSG
      Servers must be either in the format 'host:port[:weight]' (e.g., 'localhost:11211' or 'localhost:11211:10') for a network server, or a valid path to a Unix domain socket (e.g., /var/run/memcached).
But it was #{servers.inspect}.
          MSG
    end

    def normalize_behaviors(options)
      # UDP requires noreply
      options[:noreply] = true if options[:use_udp]

      # Buffering requires non-blocking
      # FIXME This should all be wrapped up in a single :pipeline option.
      options[:no_block] = true if options[:buffer_requests]

      # Disallow weights without ketama
      options.delete(:ketama_weighted) if options[:distribution] != :consistent_ketama

      # Disallow :sort_hosts with consistent hashing
      if options[:sort_hosts] && options[:distribution] == :consistent
        raise ArgumentError, ":sort_hosts defeats :consistent hashing"
      end

      if timeout = options.delete(:timeout)
        options[:rcv_timeout] ||= timeout
        options[:snd_timeout] ||= timeout
        options[:poll_timeout] ||= timeout
      end
      options
    end

  end
end
