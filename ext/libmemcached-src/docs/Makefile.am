INCLUDES = include

EXTRA_DIST = libmemcached.pod\
	memcached_flush.pod\
	memcached_stats.pod\
	memrm.pod\
	memerror.pod\
	libmemcached_examples.pod\
	memcached_get.pod\
	memcached_strerror.pod\
	memslap.pod\
	memcached_auto.pod\
	memcached_quit.pod\
	memcached_verbosity.pod\
	memstat.pod\
	memcached_behavior.pod\
	memcached_callback.pod\
	memcached_server_st.pod\
	memcat.pod\
	memcached_create.pod\
	memcached_servers.pod\
	memcp.pod\
	memcached_delete.pod\
	memcached_set.pod\
	memcached_version.pod\
	memflush.pod

man_MANS = libmemcached.3\
	libmemcached_examples.3\
	memcat.1\
	memcp.1\
	memerror.1\
	memflush.1\
	memrm.1\
	memslap.1\
	memstat.1\
	memcached_add.3\
	memcached_add_by_key.3\
	memcached_append.3\
	memcached_append_by_key.3\
	memcached_behavior_get.3\
	memcached_behavior_set.3\
	memcached_callback_get.3\
	memcached_callback_set.3\
	memcached_cas.3\
	memcached_cas_by_key.3\
	memcached_clone.3\
	memcached_create.3\
	memcached_decrement.3\
	memcached_delete.3\
	memcached_delete_by_key.3\
	memcached_fetch.3\
	memcached_fetch_execute.3\
	memcached_fetch_result.3\
	memcached_free.3\
	memcached_get.3\
	memcached_get_by_key.3\
	memcached_increment.3\
	memcached_mget.3\
	memcached_mget_by_key.3\
	memcached_prepend.3\
	memcached_prepend_by_key.3\
	memcached_replace.3\
	memcached_replace_by_key.3\
	memcached_server_add.3\
	memcached_server_count.3\
	memcached_server_list.3\
	memcached_server_list_append.3\
	memcached_server_list_count.3\
	memcached_server_list_free.3\
	memcached_server_push.3\
	memcached_servers_parse.3\
	memcached_set.3\
	memcached_set_by_key.3\
	memcached_stat.3\
	memcached_stat_get_keys.3\
	memcached_stat_get_value.3\
	memcached_stat_servername.3\
	memcached_strerror.3\
	memcached_quit.3\
	memcached_verbosity.3\
	memcached_lib_version.3\
	memcached_version.3

libmemcached.3: libmemcached.pod 
	@POD2MAN@ -c "libmemcached" -r "" -s 3 libmemcached.pod > libmemcached.3

libmemcached_examples.3: libmemcached_examples.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 libmemcached_examples.pod > libmemcached_examples.3

memcached_create.3: memcached_create.pod 
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_create.pod > memcached_create.3

memcached_free.3: memcached_create.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_create.pod > memcached_free.3

memcached_clone.3: memcached_create.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_create.pod > memcached_clone.3

memcached_set.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_set.3

memcached_set_by_key.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_set_by_key.3

memcached_cas.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_cas.3

memcached_cas_by_key.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_cas_by_key.3

memcached_replace.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_replace.3

memcached_replace_by_key.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_replace_by_key.3

memcached_add.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_add.3

memcached_add_by_key.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_add_by_key.3

memcached_prepend.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_prepend.3

memcached_prepend_by_key.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_prepend_by_key.3

memcached_append.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_append.3

memcached_append_by_key.3: memcached_set.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_set.pod > memcached_append_by_key.3

memcached_delete.3: memcached_delete.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_delete.pod > memcached_delete.3

memcached_delete_by_key.3: memcached_delete.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_delete.pod > memcached_delete_by_key.3

memcached_increment.3:
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_auto.pod > memcached_increment.3

memcached_decrement.3:
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_auto.pod > memcached_decrement.3

memcached_flush.3: memcached_flush.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_flush.pod > memcached_flush.3

memcached_get.3: memcached_get.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_get.pod > memcached_get.3

memcached_get_by_key.3: memcached_get.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_get.pod > memcached_get_by_key.3

memcached_fetch_result.3: memcached_get.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_get.pod > memcached_fetch_result.3

memcached_fetch_execute.3: memcached_get.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_get.pod > memcached_fetch_execute.3

memcached_mget.3: memcached_get.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_get.pod > memcached_mget.3

memcached_mget_by_key.3: memcached_get.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_get.pod > memcached_mget_by_key.3

memcached_fetch.3: memcached_get.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_get.pod > memcached_fetch.3

memcached_quit.3: memcached_quit.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_quit.pod > memcached_quit.3

memcached_strerror.3: memcached_strerror.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_strerror.pod > memcached_strerror.3

memcached_server_count.3: memcached_servers.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_servers.pod > memcached_server_count.3

memcached_server_list.3: memcached_servers.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_servers.pod > memcached_server_list.3

memcached_server_add.3: memcached_servers.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_servers.pod > memcached_server_add.3

memcached_server_add_unix_socket.3: memcached_servers.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_servers.pod > memcached_server_add_unix_socket.3

memcached_server_push.3: memcached_servers.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_servers.pod > memcached_server_push.3

memcached_server_list_free.3: memcached_server_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_server_st.pod > memcached_server_list_free.3

memcached_server_list_count.3: memcached_server_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_server_st.pod > memcached_server_list_count.3

memcached_server_list_append.3: memcached_server_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_server_st.pod > memcached_server_list_append.3

memcached_servers_parse.3: memcached_server_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_server_st.pod > memcached_servers_parse.3

memcached_verbosity.3: memcached_verbosity.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_verbosity.pod > memcached_verbosity.3

memcached_behavior_get.3: memcached_behavior.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_behavior.pod > memcached_behavior_get.3

memcached_behavior_set.3: memcached_behavior.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_behavior.pod > memcached_behavior_set.3

memcached_callback_get.3: memcached_callback.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_callback.pod > memcached_callback_get.3

memcached_callback_set.3: memcached_callback.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_callback.pod > memcached_callback_set.3

memcached_stat.3: memcached_stats.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_stats.pod > memcached_stat.3

memcached_stat_servername.3: memcached_stats.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_stats.pod > memcached_stat_servername.3

memcached_stat_get_value.3: memcached_stats.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_stats.pod > memcached_stat_get_value.3

memcached_stat_get_keys.3: memcached_stats.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_stats.pod > memcached_stat_get_keys.3

memcached_result_st.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_st.3

memcached_result_create.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_create.3

memcached_result_free.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_free.3

memcached_result_key_value.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_key_value.3

memcached_result_key_length.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_key_length.3

memcached_result_value.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_value.3

memcached_result_length.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_length.3

memcached_result_flags.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_flags.3

memcached_result_cas.3: memcached_result_st.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_result_st.pod > memcached_result_cas.3

memcached_version.3: memcached_version.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_version.pod > memcached_version.3

memcached_lib_version.3: memcached_version.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 3 memcached_version.pod  > memcached_lib_version.3

memcp.1: memcp.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 1 memcp.pod > memcp.1

memslap.1: memslap.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 1 memslap.pod > memslap.1

memcat.1: memcat.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 1 memcat.pod > memcat.1

memstat.1: memstat.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 1 memstat.pod > memstat.1

memrm.1: memrm.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 1 memrm.pod > memrm.1

memerror.1: memerror.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 1 memerror.pod > memerror.1

memflush.1: memflush.pod
	@POD2MAN@ -c "libmemcached" -r "" -s 1 memflush.pod > memflush.1

test: 
	podchecker libmemcached.pod
	podchecker memcached_flush.pod
	podchecker memcached_stats.pod
	podchecker memrm.pod
	podchecker memerror.pod
	podchecker libmemcached_examples.pod
	podchecker memcached_get.pod
	podchecker memcached_strerror.pod
	podchecker memslap.pod
	podchecker memcached_auto.pod
	podchecker memcached_quit.pod
	podchecker memcached_verbosity.pod
	podchecker memstat.pod
	podchecker memcached_behavior.pod
	podchecker memcached_callback.pod
	podchecker memcached_server_st.pod
	podchecker memcat.pod
	podchecker memcached_create.pod
	podchecker memcached_servers.pod
	podchecker memcp.pod
	podchecker memcached_delete.pod
	podchecker memcached_set.pod
	podchecker memcached_version.pod
	podchecker memflush.pod


html:
	pod2htmltree "/libmemcached" .

clean:
	rm -f *.1
	rm -f *.3

distclean:
	rm -f *.1
	rm -f *.3
	rm -f Makefile
