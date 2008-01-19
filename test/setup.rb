
# Start memcached

`ps awx`.split("\n").grep(/4304[1-3]/).map do |process| 
  system("kill -9 #{process.to_i}")
end

system "memcached -p 43042 &"
system "memcached -p 43043 &"  
