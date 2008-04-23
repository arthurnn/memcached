
# Start memcached

HERE = File.dirname(__FILE__)

`ps awx`.split("\n").grep(/4304[2-6]/).map do |process| 
  system("kill -9 #{process.to_i}")
end

log = "/tmp/memcached.log"
system ">#{log}"

verbosity = (ENV['DEBUG'] ? "-vv" : "")

(43042..43046).each do |port|
  system "memcached #{verbosity} -p #{port} >> #{log} 2>&1 &"
end