
# Start memcached

HERE = File.dirname(__FILE__)
UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached')
NB_UNIX_DOMAIN_SOCKET = 2

`ps awx`.split("\n").grep(/4304[2-6]/).map do |process| 
  system("kill -9 #{process.to_i}")
end

log = "/tmp/memcached.log"
system ">#{log}"

verbosity = (ENV['DEBUG'] ? "-vv" : "")

# networked memcached
(43042..43046).each do |port|
  system "memcached #{verbosity} -p #{port} >> #{log} 2>&1 &"
end

# unix domain sockets memcached
0.upto NB_UNIX_DOMAIN_SOCKET-1 do |i|
  # kill remaining memcached using the unix domain socket
  pid = `lsof -t -a -U -c memcached #{UNIX_SOCKET_NAME}#{i}`
  system("kill -9 #{pid}") unless pid.to_i.zero?
  system "memcached -M -s #{UNIX_SOCKET_NAME}#{i} #{verbosity} >> #{log} 2>&1 &"
end