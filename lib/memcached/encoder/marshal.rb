class Memcached
  class Encoder
    class Marshal
      def self.encode(value)
        ::Marshal.dump(value)
      end
      def self.decode(value)
        ::Marshal.load(value)
      end
    end
  end
end
