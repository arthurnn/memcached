
class Memcached

  FLAGS = 0x0

  attr_reader :namespace

  def initialize(servers, opts = {})
    @struct = Libmemcached::MemcachedSt.new
    Libmemcached.memcached_create(@struct)

    Array(servers).each do |server|
      unless server.is_a? String and server =~ /^(\d{1,3}\.){3}\d{1,3}:\d{1,5}$/
        raise ArgumentError, "Servers must be in the format ip:port (e.g., '127.0.0.1:11211')" 
      end
      host, port = server.split(":")
      Libmemcached.memcached_server_add(@struct, host, port.to_i)
    end  
    @namespace = opts[:namespace]
  end
  
  def servers
    servers = []
    @struct.hosts.count.times do |i|
      servers << Libmemcached.memcached_select_server_at(@struct, i)
    end
    servers
  end
  
  def set(key, value, timeout=0, raw=false)
    value = Marshal.dump(value) unless raw
    check_return_code(
      Libmemcached.memcached_set(@struct, key, value, timeout, FLAGS)
    )
  end
  
  def get(key, raw=false, exception=false)
    raise ClientError, "Invalid key" if key =~ /\s/ # XXX Server doesn't validate. Possibly a performance problem.
    value, flags, return_code = Libmemcached.memcached_get_ruby_string(@struct, key)
    STDERR.puts [value, flags, return_code].inspect
    check_return_code(return_code)
    value = Marshal.load(value) unless raw
    value
  end
  
  def add
  end
  
  def incr
  end
  
  def decr
  end
  
  alias :increment :incr
  alias :decrement :decr
  
  def stats
  end  
  
  private
  
  def check_return_code(int)
    return true if int == 0
    raise @@exceptions[int]
  end  
    
end

#>> pp Libmemcached.constants.grep /cache/
#(irb):5: warning: parenthesize argument(s) for future version
#["MemcachedServerSt",
# "MemcachedStatSt",
# "MemcachedStringSt",
# "MemcachedResultSt",
# "MemcachedSt"]
#=> nil

#>> pp Libmemcached.local_methods
#["<",
# "<=",
# "<=>",
# ">",
# ">=",
# "ancestors",
# "autoload",
# "autoload?",
# "class_eval",
# "class_variable_defined?",
# "class_variables",
# "const_defined?",
# "const_get",
# "const_missing",
# "const_set",
# "constants",
# "debug_method",
# "include?",
# "included_modules",
# "instance_method",
# "instance_methods",
# "memcached_add",
# "memcached_add_by_key",
# "memcached_append",
# "memcached_append_by_key",
# "memcached_behavior_get",
# "memcached_behavior_set",
# "memcached_cas",
# "memcached_cas_by_key",
# "memcached_clone",
# "memcached_create",
# "memcached_decrement",
# "memcached_delete",
# "memcached_delete_by_key",
# "memcached_fetch",
# "memcached_fetch_execute",
# "memcached_fetch_result",
# "memcached_flush",
# "memcached_free",
# "memcached_get",
# "memcached_get_by_key",
# "memcached_increment",
# "memcached_mget",
# "memcached_mget_by_key",
# "memcached_prepend",
# "memcached_prepend_by_key",
# "memcached_quit",
# "memcached_replace",
# "memcached_replace_by_key",
# "memcached_result_create",
# "memcached_result_free",
# "memcached_result_length",
# "memcached_result_value",
# "memcached_server_add",
# "memcached_server_add_udp",
# "memcached_server_add_unix_socket",
# "memcached_server_list_append",
# "memcached_server_list_count",
# "memcached_server_list_free",
# "memcached_server_push",
# "memcached_servers_parse",
# "memcached_set",
# "memcached_set_by_key",
# "memcached_stat",
# "memcached_stat_free",
# "memcached_stat_get_keys",
# "memcached_stat_get_value",
# "memcached_stat_servername",
# "memcached_strerror",
# "memcached_verbosity",
# "method_defined?",
# "module_eval",
# "name",
# "post_mortem_method",
# "private_class_method",
# "private_instance_methods",
# "private_method_defined?",
# "protected_instance_methods",
# "protected_method_defined?",
# "public_class_method",
# "public_instance_methods",
# "public_method_defined?"]
#=> nil