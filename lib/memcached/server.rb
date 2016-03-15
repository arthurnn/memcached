module Memcached
  class Server
    attr_reader :hostname
    attr_reader :port
    attr_reader :weight

    def initialize(name, port, weight = nil)
      @hostname = name
      @port = port
      @weight = weight
    end

    def to_s
      "#{hostname}:#{port}"
    end
  end
end
