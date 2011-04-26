CLEANFILES= *.1 *.3


EXTRA_DIST = libmemcached.pod\
	libmemcachedutil.pod\
	memcached_flush.pod\
	memcached_dump.pod\
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
	memcached_pool.pod\
	memcached_servers.pod\
	memcp.pod\
	memcached_delete.pod\
	memcached_set.pod\
	memcached_version.pod\
	memflush.pod\
	memdump.pod\
	memcached_flush_buffers.pod\
        memcached_analyze.pod\
        memcached_generate_hash_value.pod\
        memcached_memory_allocators.pod\
        memcached_user_data.pod

man_MANS = libmemcached.3\
	libmemcached_examples.3\
	memcat.1\
	memcp.1\
	memerror.1\
	memflush.1\
	memdump.1\
	memrm.1\
	memslap.1\
	memstat.1\
	memcached_add.3\
	memcached_add_by_key.3\
	memcached_append.3\
	memcached_append_by_key.3\
	memcached_analyze.3\
	memcached_behavior_get.3\
	memcached_behavior_set.3\
	memcached_callback_get.3\
	memcached_callback_set.3\
	memcached_cas.3\
	memcached_cas_by_key.3\
	memcached_clone.3\
	memcached_create.3\
	memcached_decrement.3\
	memcached_decrement_with_initial.3\
	memcached_delete.3\
	memcached_delete_by_key.3\
	memcached_fetch.3\
	memcached_fetch_execute.3\
	memcached_fetch_result.3\
	memcached_free.3\
	memcached_get.3\
	memcached_get_by_key.3\
	memcached_increment.3\
	memcached_increment_with_initial.3\
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
	memcached_version.3\
        memcached_flush_buffers.3\
        memcached_dump.3\
        memcached_generate_hash_value.3\
        memcached_set_memory_allocators.3\
        memcached_get_memory_allocators.3\
        memcached_set_user_data.3\
        memcached_get_user_data.3

if BUILD_LIBMEMCACHEDUTIL
man_MANS+= libmemcachedutil.3 \
	memcached_pool_create.3 \
	memcached_pool_destroy.3 \
	memcached_pool_push.3 \
	memcached_pool_pop.3
endif


libmemcached.3: libmemcached.pod 
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/libmemcached.pod > libmemcached.3

libmemcachedutil.3: libmemcachedutil.pod 
	${POD2MAN} -c "libmemcachedutil" -r "" -s 3 ${top_srcdir}/docs/libmemcachedutil.pod > libmemcachedutil.3

libmemcached_examples.3: libmemcached_examples.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/libmemcached_examples.pod > libmemcached_examples.3

memcached_create.3: memcached_create.pod 
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_create.pod > memcached_create.3

memcached_free.3: memcached_create.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_create.pod > memcached_free.3

memcached_clone.3: memcached_create.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_create.pod > memcached_clone.3

memcached_set.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_set.3

memcached_set_by_key.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_set_by_key.3

memcached_cas.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_cas.3

memcached_cas_by_key.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_cas_by_key.3

memcached_replace.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_replace.3

memcached_replace_by_key.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_replace_by_key.3

memcached_add.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_add.3

memcached_add_by_key.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_add_by_key.3

memcached_prepend.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_prepend.3

memcached_prepend_by_key.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_prepend_by_key.3

memcached_append.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_append.3

memcached_append_by_key.3: memcached_set.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_set.pod > memcached_append_by_key.3

memcached_delete.3: memcached_delete.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_delete.pod > memcached_delete.3

memcached_delete_by_key.3: memcached_delete.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_delete.pod > memcached_delete_by_key.3

memcached_increment.3:
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_auto.pod > memcached_increment.3

memcached_increment_with_initial.3:
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_auto.pod > memcached_increment_with_initial.3

memcached_decrement.3:
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_auto.pod > memcached_decrement.3

memcached_decrement_with_initial.3:
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_auto.pod > memcached_decrement_with_initial.3

memcached_dump.3: memcached_dump.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_dump.pod > memcached_dump.3

memcached_flush.3: memcached_flush.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_flush.pod > memcached_flush.3

memcached_get.3: memcached_get.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_get.pod > memcached_get.3

memcached_get_by_key.3: memcached_get.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_get.pod > memcached_get_by_key.3

memcached_fetch_result.3: memcached_get.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_get.pod > memcached_fetch_result.3

memcached_fetch_execute.3: memcached_get.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_get.pod > memcached_fetch_execute.3

memcached_mget.3: memcached_get.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_get.pod > memcached_mget.3

memcached_mget_by_key.3: memcached_get.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_get.pod > memcached_mget_by_key.3

memcached_fetch.3: memcached_get.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_get.pod > memcached_fetch.3

memcached_quit.3: memcached_quit.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_quit.pod > memcached_quit.3

memcached_strerror.3: memcached_strerror.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_strerror.pod > memcached_strerror.3

memcached_server_count.3: memcached_servers.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_servers.pod > memcached_server_count.3

memcached_server_list.3: memcached_servers.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_servers.pod > memcached_server_list.3

memcached_server_add.3: memcached_servers.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_servers.pod > memcached_server_add.3

memcached_server_add_unix_socket.3: memcached_servers.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_servers.pod > memcached_server_add_unix_socket.3

memcached_server_push.3: memcached_servers.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_servers.pod > memcached_server_push.3

memcached_server_list_free.3: memcached_server_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_server_st.pod > memcached_server_list_free.3

memcached_server_list_count.3: memcached_server_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_server_st.pod > memcached_server_list_count.3

memcached_server_list_append.3: memcached_server_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_server_st.pod > memcached_server_list_append.3

memcached_servers_parse.3: memcached_server_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_server_st.pod > memcached_servers_parse.3

memcached_verbosity.3: memcached_verbosity.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_verbosity.pod > memcached_verbosity.3

memcached_behavior_get.3: memcached_behavior.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_behavior.pod > memcached_behavior_get.3

memcached_behavior_set.3: memcached_behavior.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_behavior.pod > memcached_behavior_set.3

memcached_callback_get.3: memcached_callback.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_callback.pod > memcached_callback_get.3

memcached_callback_set.3: memcached_callback.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_callback.pod > memcached_callback_set.3

memcached_stat.3: memcached_stats.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_stats.pod > memcached_stat.3

memcached_stat_servername.3: memcached_stats.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_stats.pod > memcached_stat_servername.3

memcached_stat_get_value.3: memcached_stats.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_stats.pod > memcached_stat_get_value.3

memcached_stat_get_keys.3: memcached_stats.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_stats.pod > memcached_stat_get_keys.3

memcached_result_st.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_st.3

memcached_result_create.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_create.3

memcached_result_free.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_free.3

memcached_result_key_value.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_key_value.3

memcached_result_key_length.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_key_length.3

memcached_result_value.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_value.3

memcached_result_length.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_length.3

memcached_result_flags.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_flags.3

memcached_result_cas.3: memcached_result_st.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_result_st.pod > memcached_result_cas.3

memcached_version.3: memcached_version.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_version.pod > memcached_version.3

memcached_lib_version.3: memcached_version.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_version.pod  > memcached_lib_version.3

memcached_flush_buffers.3: memcached_flush_buffers.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_flush_buffers.pod  > memcached_flush_buffers.3

memcached_analyze.3: memcached_analyze.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_analyze.pod > memcached_analyze.3

memcached_generate_hash_value.3: memcached_generate_hash_value.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_generate_hash_value.pod > memcached_generate_hash_value.3

memcached_get_memory_allocators.3: memcached_memory_allocators.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_memory_allocators.pod > memcached_get_memory_allocators.3

memcached_set_memory_allocators.3: memcached_memory_allocators.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_memory_allocators.pod > memcached_set_memory_allocators.3

memcached_get_user_data.3: memcached_user_data.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_user_data.pod > memcached_get_user_data.3

memcached_set_user_data.3: memcached_user_data.pod
	${POD2MAN} -c "libmemcached" -r "" -s 3 ${top_srcdir}/docs/memcached_user_data.pod > memcached_set_user_data.3

memcached_pool_create.3: memcached_pool.pod 
	${POD2MAN} -c "libmemcachedutil" -r "" -s 3 ${top_srcdir}/docs/memcached_pool.pod > memcached_pool_create.3

memcached_pool_destroy.3: memcached_pool.pod 
	${POD2MAN} -c "libmemcachedutil" -r "" -s 3 ${top_srcdir}/docs/memcached_pool.pod > memcached_pool_destroy.3

memcached_pool_pop.3: memcached_pool.pod 
	${POD2MAN} -c "libmemcachedutil" -r "" -s 3 ${top_srcdir}/docs/memcached_pool.pod > memcached_pool_pop.3

memcached_pool_push.3: memcached_pool.pod 
	${POD2MAN} -c "libmemcachedutil" -r "" -s 3 ${top_srcdir}/docs/memcached_pool.pod > memcached_pool_push.3

memcp.1: memcp.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memcp.pod > memcp.1

memslap.1: memslap.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memslap.pod > memslap.1

memcat.1: memcat.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memcat.pod > memcat.1

memstat.1: memstat.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memstat.pod > memstat.1

memrm.1: memrm.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memrm.pod > memrm.1

memerror.1: memerror.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memerror.pod > memerror.1

memdump.1: memdump.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memdump.pod > memdump.1

memflush.1: memflush.pod
	${POD2MAN} -c "libmemcached" -r "" -s 1 ${top_srcdir}/docs/memflush.pod > memflush.1

test: 
	podchecker $(top_srcdir)/docs/libmemcached.pod
	podchecker $(top_srcdir)/docs/memcached_flush.pod
	podchecker $(top_srcdir)/docs/memcached_stats.pod
	podchecker $(top_srcdir)/docs/memrm.pod
	podchecker $(top_srcdir)/docs/memerror.pod
	podchecker $(top_srcdir)/docs/libmemcached_examples.pod
	podchecker $(top_srcdir)/docs/memcached_get.pod
	podchecker $(top_srcdir)/docs/memcached_strerror.pod
	podchecker $(top_srcdir)/docs/memslap.pod
	podchecker $(top_srcdir)/docs/memcached_auto.pod
	podchecker $(top_srcdir)/docs/memcached_quit.pod
	podchecker $(top_srcdir)/docs/memcached_verbosity.pod
	podchecker $(top_srcdir)/docs/memstat.pod
	podchecker $(top_srcdir)/docs/memcached_behavior.pod
	podchecker $(top_srcdir)/docs/memcached_callback.pod
	podchecker $(top_srcdir)/docs/memcached_server_st.pod
	podchecker $(top_srcdir)/docs/memcat.pod
	podchecker $(top_srcdir)/docs/memcached_create.pod
	podchecker $(top_srcdir)/docs/memcached_servers.pod
	podchecker $(top_srcdir)/docs/memcp.pod
	podchecker $(top_srcdir)/docs/memcached_delete.pod
	podchecker $(top_srcdir)/docs/memcached_set.pod
	podchecker $(top_srcdir)/docs/memcached_version.pod
	podchecker $(top_srcdir)/docs/memflush.pod
	podchecker $(top_srcdir)/docs/memcached_flush_buffers.pod
	podchecker $(top_srcdir)/docs/memcached_pool.pod
	podchecker $(top_srcdir)/docs/memcached_memory_allocators.pod
	podchecker $(top_srcdir)/docs/memcached_user_data.pod

html-local:
	pod2htmltree "/libmemcached" .

