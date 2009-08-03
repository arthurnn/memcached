 
HERE = File.dirname(__FILE__)
UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached')

# Kill memcached
system("killall -9 memcached")

# Start memcached
verbosity = (ENV['DEBUG'] ? "-vv" : "")
log = "/tmp/memcached.log"
system ">#{log}"

# Network memcached
(43042..43046).each do |port|
  system "memcached #{verbosity} -U #{port} -p #{port} >> #{log} 2>&1 &"
end
# Domain socket memcached
(0..1).each do |i|
  system "memcached -M -s #{UNIX_SOCKET_NAME}#{i} #{verbosity} >> #{log} 2>&1 &"
end