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

    attr_reader :config, :behaviors

    def initialize(config = "localhost:11211", options = {})
      options = DEFAULTS.merge(options)

      @codec = Memcached::MarshalCodec
      @default_ttl = options.delete(:ttl) || 0
      @default_weight = options.delete(:default_weight) || 1
      @prefix = options.delete(:prefix_key)

      @connection = nil
      @behaviors = normalize_behaviors(options)
      @config = create_config_str(config)
      Memcached::Connection.check_config!(@config)
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

    def reset
      if @connection
        @connection.close
        @connection = nil
      end
    end

    def connection
      @connection ||= Memcached::Connection.new(@config).tap do |conn|
        conn.set_prefix(@prefix)
        conn.set_behaviors(@behaviors)
      end
    end

    private
    def create_config_str(servers, extra = nil)
      if servers.is_a?(String)
        return servers if servers.include? '--'
        servers = [servers]
      end

      raise ArgumentError unless servers.is_a?(Array)

      config = servers.map do |server|
        server = server.to_s
        if File.socket?(server)
          "--SOCKET=\"#{server}\""
        else
          host, port, weight = server.split(":")
          port = (port && !port.empty?) ? ":#{port}" : ""
          weight = (weight && !weight.empty?) ? "/?#{weight}" : ""
          "--SERVER=#{host}#{port}#{weight}"
        end
      end.join(' ')
      config << " #{extra.strip}" if extra
      config << " --VERIFY-KEY" unless config.include? "--VERIFY-KEY"
      config
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
