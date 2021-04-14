require 'memcached/behaviors'

module Memcached
  class Connection
    include Behaviors
  end

  class Client
    FLAGS = 0b0
    FLAG_ENCODED = 0b1

    require 'memcached/marshal_codec'

    DEFAULTS = {
      :hash => :fnv1_32,
      :distribution => :consistent_ketama,
      :ketama_weighted => true,
      :verify_key => true,
    }

    attr_reader :config, :behaviors

    def initialize(config = "localhost:11211", options = {})
      options = DEFAULTS.merge(options)

      @codec = options.delete(:codec) || Memcached::MarshalCodec
      @default_ttl = options.delete(:ttl) || 0
      @prefix = options.delete(:prefix_key)
      @credentials = options.delete(:credentials)
      if !@credentials && ENV["MEMCACHE_USERNAME"] && ENV["MEMCACHE_PASSWORD"]
        @credentials = [ENV["MEMCACHE_USERNAME"], ENV["MEMCACHE_PASSWORD"]]
      end
      # SASL requires binary protocol
      options[:binary_protocol] = true if @credentials

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

    def get(key, raw: nil)
      value, flags = connection.get(key)
      return nil unless value
      if raw != true
        value = @codec.decode(key, value, flags)
      end

      value
    end

    def get_multi(keys, raw: nil)
      keys = keys.compact
      hash = connection.get_multi(keys)
      hash.each do |key, (value, flags)|
        if raw != true
          hash[key] = @codec.decode(key, value, flags)
        else
          hash[key] = value
        end
      end
      hash
    end

    def cas(keys, ttl: @default_ttl, raw: nil)
      responses = connection.get_multi(keys)
      return false if responses.empty?

      values_hash = if raw
        responses.transform_values(&:first)
      else
        hash = responses.dup
        hash.each do |key, (raw_value, flags)|
          hash[key] = @codec.decode(key, raw_value, flags)
        end
      end
      values_hash = yield values_hash

      success = true
      responses.each do |key, (_orig_value, flags, cas)|
        if values_hash.key?(key)
          new_value = values_hash[key]
          unless raw
            new_value, flags = @codec.encode(key, new_value, flags)
          end
          success &= connection.cas(key, new_value, ttl, flags, cas)
        else
          success = false
        end
      end
      success
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
      @prefix
    end

    def namespace=(value)
      connection.set_prefix(value)
      @prefix = value
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
        conn.set_credentials(*@credentials) if @credentials
      end
    end

    private
    def create_config_str(servers)
      if servers.is_a?(String)
        return servers if servers.include? '--'
        servers = [servers]
      end

      raise ArgumentError unless servers.is_a?(Array)

      servers.map do |server|
        server = server.to_s
        hostname = server.gsub(/\/\?\d+$/, '')

        if hostname =~ /^[\w\.-]+(:\d{1,5})?$/
          "--SERVER=#{server}"
        elsif File.socket?(hostname)
          "--SOCKET=\"#{server}\""
        else
          raise ArgumentError, "not a valid server address: #{server}"
        end
      end.join(' ')
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
