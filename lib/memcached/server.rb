module Memcached
  class Server
    attr_reader :name
    attr_reader :port
    attr_reader :weight

    def initialize(name, port, weight = nil)
      @name = name
      @port = port
      @weight = weight
    end

    def to_s
      "#{name}:#{port}"
    end
  end
end
