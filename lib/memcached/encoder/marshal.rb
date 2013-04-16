class Memcached
  class Encoder
    class Marshal
      def self.encode(key, value, flags)
        [::Marshal.dump(value), flags]
      end
      def self.decode(key, value, flags)
        ::Marshal.load(value)
      end
    end
  end
end
