
# Start memcached

HERE = File.dirname(__FILE__)
UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached') 

`ps awx`.split("\n").grep(/4304[2-6]/).map do |process| 
  system("kill -9 #{process.to_i}")
end

# kill remaining memcached using the unix socket
system("kill -9 `lsof -t -a -U -c memcached #{UNIX_SOCKET_NAME}`")

log = "/tmp/memcached.log"
system ">#{log}"

verbosity = (ENV['DEBUG'] ? "-vv" : "")

# networked memcached
(43042..43046).each do |port|
  system "memcached #{verbosity} -p #{port} >> #{log} 2>&1 &"
end

# unix domain sockets memcached
system "memcached -s #{UNIX_SOCKET_NAME} #{verbosity} >> #{log} 2>&1 &"