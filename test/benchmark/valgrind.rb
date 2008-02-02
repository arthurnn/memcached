

# At 2000 loops:
#
#==18554== LEAK SUMMARY:
#==18554==    definitely lost: 34,128 bytes in 2 blocks.
#==18554==    indirectly lost: 64 bytes in 1 blocks.
#==18554==      possibly lost: 32 bytes in 1 blocks.
#==18554==    still reachable: 764,457 bytes in 10,198 blocks.
#==18554==         suppressed: 0 bytes in 0 blocks.
#==18554== Reachable blocks (those to which a pointer was found) are not shown.
#==18554== To see them, rerun with: --leak-check=full --show-reachable=yes
#
# At 5000 loops:
#
#==18693== LEAK SUMMARY:
#==18693==    definitely lost: 34,128 bytes in 2 blocks.
#==18693==    indirectly lost: 64 bytes in 1 blocks.
#==18693==      possibly lost: 32 bytes in 1 blocks.
#==18693==    still reachable: 764,457 bytes in 10,198 blocks.
#==18693==         suppressed: 0 bytes in 0 blocks.
#==18693== Reachable blocks (those to which a pointer was found) are not shown.
#==18693== To see them, rerun with: --leak-check=full --show-reachable=yes
#
# Conclusion: no leaks.

HERE = File.dirname(__FILE__)
$LOAD_PATH << "#{HERE}/../../lib/"

require 'memcached'

class Worker  
  def initialize
    @key = "key-"*8  
    @value = []
    @marshalled = Marshal.dump(@value)
    @opts = [
      ['127.0.0.1:43042', '127.0.0.1:43043'], 
      {
        :buffer_requests => true,
        :no_block => true,
        :namespace => "benchmark_namespace"
      }
    ]    
    system("ruby #{HERE}/../setup.rb")
    sleep(1)  
    @m = Memcached.new(*@opts)
  end
  
  def work
    @m.set @key, @value
    @m.set @key, @value
    @m.set @key, @value
    @m.get @key
    @m.get @key
    @m.get @key
    @m.set @key, @value
    @m.get @key
    @m.set @key, @value
    @m.get @key
    @m.set @key, @value
    @m.get @key
  end  
end

worker = Worker.new

2000.times do
  worker.work
  GC.start
end


