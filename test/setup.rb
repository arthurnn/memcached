
unless defined? UNIX_SOCKET_NAME
  HERE = File.dirname(__FILE__)
  UNIX_SOCKET_NAME = File.join(ENV['TMPDIR']||'/tmp','memcached')

  # Kill memcached
  system("killall -9 memcached")

  # Start memcached
  verbosity = (ENV['DEBUG'] ? "-vv" : "")
  log = "/tmp/memcached.log"
  memcached = ENV['MEMCACHED_COMMAND'] || 'memcached'
  system ">#{log}"

  # TCP memcached
  (43042..43046).each do |port|
    cmd = "#{memcached} #{verbosity} -U 0 -p #{port} >> #{log} 2>&1 &"
    raise "'#{cmd}' failed to start" unless system(cmd)
  end
  # UDP memcached
  (43052..43053).each do |port|
    cmd = "#{memcached} #{verbosity} -U #{port} -p 0 >> #{log} 2>&1 &"
    raise "'#{cmd}' failed to start" unless system(cmd)
  end
  # Domain socket memcached
  (0..1).each do |i|
    cmd = "#{memcached} -M -s #{UNIX_SOCKET_NAME}#{i} #{verbosity} >> #{log} 2>&1 &"
    raise "'#{cmd}' failed to start" unless system(cmd)
  end
end
