
class Memcached

  attr_reader :namespace

  def initialize(servers, opts = {})
    @struct = Libmemcached::MemcachedSt.new

    Array(servers).each do |server|
      raise ArgumentError, "Servers must be Strings" unless server.is_a? String
      host, port = server.split(":")
      Libmemcached.memcached_server_add(@struct, host, port.to_i)
    end  
    @namespace = opts[:namespace]
  end
  
  def servers
    (0...@struct.hosts.count).inject([]) do |servers, i|
      server = Libmemcached.memcached_select_server_at(@struct, i)
      servers << "#{server.hostname}:#{server.port}"
    end
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