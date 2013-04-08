class Memcached
  class Encoder
    class Marshal
      def self.encode(value, flags)
        ::Marshal.dump(value)
      end
      def self.decode(value, flags)
        ::Marshal.load(value)
      end
    end
  end
end
