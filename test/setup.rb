
# Start memcached

HERE = File.dirname(__FILE__)

`ps awx`.split("\n").grep(/4304[1-3]/).map do |process| 
  system("kill -9 #{process.to_i}")
end

log = "#{HERE}/log/memcached.log"
system "touch #{log}"

system "memcached -vv -p 43042 >> #{log} 2>&1 &"
system "memcached -vv -p 43043 >> #{log} 2>&1 &"  
