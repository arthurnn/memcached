#include "memcached_rb.h"

VALUE rb_mMemcached;

static VALUE rb_cServer;
static VALUE rb_cConnection;
static VALUE rb_eMemcachedError;

static ID id_tcp;
static ID id_socket;

const char *MEMCACHED_ERROR_NAMES[] = {
	NULL, // MEMCACHED_SUCCESS
	"Failure", // MEMCACHED_FAILURE
	"HostLookupFailure", // MEMCACHED_HOST_LOOKUP_FAILURE
	"ConnectionFailure", // MEMCACHED_CONNECTION_FAILURE
	"ConnectionBindFailure", // MEMCACHED_CONNECTION_BIND_FAILURE
	"WriteFailure", // MEMCACHED_WRITE_FAILURE
	"ReadFailure", // MEMCACHED_READ_FAILURE
	"UnknownReadFailure", // MEMCACHED_UNKNOWN_READ_FAILURE
	"ProtocolError", // MEMCACHED_PROTOCOL_ERROR
	"ClientError", // MEMCACHED_CLIENT_ERROR
	"ServerError", // MEMCACHED_SERVER_ERROR
	"StdError", // MEMCACHED_ERROR
	"DataExist", // MEMCACHED_DATA_EXISTS
	"DataDoesNotExist", // MEMCACHED_DATA_DOES_NOT_EXIST
	NULL, // MEMCACHED_NOTSTORED
	NULL, // MEMCACHED_STORED
	NULL, // MEMCACHED_NOTFOUND
	"MemoryAllocationFailure", // MEMCACHED_MEMORY_ALLOCATION_FAILURE
	"PartialRead", // MEMCACHED_PARTIAL_READ
	"SomeError", // MEMCACHED_SOME_ERRORS
	"NoServer", // MEMCACHED_NO_SERVERS
	"End", // MEMCACHED_END
	"Deleted", // MEMCACHED_DELETED
	"Value", // MEMCACHED_VALUE
	"Stat", // MEMCACHED_STAT
	"Item", // MEMCACHED_ITEM
	"Errno", // MEMCACHED_ERRNO
	"FailUnixSocket", // MEMCACHED_FAIL_UNIX_SOCKET
	"NotSupported", // MEMCACHED_NOT_SUPPORTED
	"NoKeyProvided", // MEMCACHED_NO_KEY_PROVIDED
	"FetchNotfinished", // MEMCACHED_FETCH_NOTFINISHED
	"Timeout", // MEMCACHED_TIMEOUT
	NULL, // MEMCACHED_BUFFERED
	"BadKeyProvided", // MEMCACHED_BAD_KEY_PROVIDED
	"InvalidHostProtocol", // MEMCACHED_INVALID_HOST_PROTOCOL
	"ServerMarkedDead", // MEMCACHED_SERVER_MARKED_DEAD
	"UnknownStatKey", // MEMCACHED_UNKNOWN_STAT_KEY
	"E2big", // MEMCACHED_E2BIG
	"InvalidArgument", // MEMCACHED_INVALID_ARGUMENTS
	"KeyTooBig", // MEMCACHED_KEY_TOO_BIG
	"AuthProblem", // MEMCACHED_AUTH_PROBLEM
	"AuthFailure", // MEMCACHED_AUTH_FAILURE
	"AuthContinue", // MEMCACHED_AUTH_CONTINUE
	"ParseError", // MEMCACHED_PARSE_ERROR
	"ParseUserError", // MEMCACHED_PARSE_USER_ERROR
	"Deprecated", // MEMCACHED_DEPRECATED
	"InProgress", // MEMCACHED_IN_PROGRESS
	"ServerTemporarilyDisabled", // MEMCACHED_SERVER_TEMPORARILY_DISABLED
	"ServerMemoryAllocationFailure", // MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE
	"MaximumReturn", // MEMCACHED_MAXIMUM_RETURN
	"ConnectionSocketCreateFailure", // MEMCACHED_CONNECTION_SOCKET_CREATE_FAILURE
};

#define MEMCACHED_ERROR_COUNT (ARRAY_SIZE(MEMCACHED_ERROR_NAMES))
static VALUE rb_eMemcachedErrors[MEMCACHED_ERROR_COUNT];

void
rb_memcached_error_check(memcached_return_t rc)
{
	VALUE rb_err;
	const char * message;

	if (rc == MEMCACHED_SUCCESS)
		return;

	if (rc > 0 && rc < MEMCACHED_ERROR_COUNT) {
		rb_err = rb_eMemcachedErrors[rc];
		if (NIL_P(rb_err))
			return;
		message = memcached_strerror(NULL, rc);
	} else {
		rb_err = rb_eMemcachedError;
		message = "Memcached returned type not handled";
	}

	rb_exc_raise(rb_exc_new2(rb_err, message));
}

#define rb_memcached_return(rc) { \
		rb_memcached_error_check(rc); \
		switch (rc) { \
		case MEMCACHED_BUFFERED: return Qnil; \
		case MEMCACHED_SUCCESS: return Qtrue; \
		default: return Qfalse; }}


static VALUE
rb_memcached_alloc(VALUE klass)
{
	memcached_st *mc = memcached_create(NULL);
	return Data_Wrap_Struct(klass, NULL, memcached_free, mc);
}

static VALUE
rb_connection_new(VALUE klass, VALUE rb_servers)
{
	VALUE rb_mc = rb_memcached_alloc(klass);

	memcached_st *mc;
	memcached_return_t rc;
	long i;

	UnwrapMemcached(rb_mc, mc);
	Check_Type(rb_servers, T_ARRAY);

	for (i = 0; i < RARRAY_LEN(rb_servers); ++i) {
		VALUE rb_server, rb_backend;
		
		rb_server = rb_ary_entry(rb_servers, i);
		Check_Type(rb_server, T_ARRAY);

		rb_backend = rb_ary_entry(rb_server, 0);
		Check_Type(rb_backend, T_SYMBOL);

		if (SYM2ID(rb_backend) == id_tcp) {
			VALUE rb_hostname = rb_ary_entry(rb_server, 1);
			VALUE rb_port = rb_ary_entry(rb_server, 2);

			Check_Type(rb_server, T_STRING);
			Check_Type(rb_port, T_FIXNUM);

			// TODO: add weight
			//VALUE rb_weight = rb_ary_entry(rb_server, 3);
			rc = memcached_server_add(mc, StringValueCStr(rb_hostname), NUM2INT(rb_port));
			rb_memcached_error_check(rc);
		}
		else if (SYM2ID(rb_backend) == id_socket) {
			VALUE rb_fd = rb_ary_entry(rb_server, 1);
			Check_Type(rb_fd, T_STRING);
			rc = memcached_server_add_unix_socket(mc, StringValueCStr(rb_fd));
			rb_memcached_error_check(rc);
		}
	}

	// Verify keys by default
	rc = memcached_behavior_set(mc, MEMCACHED_BEHAVIOR_VERIFY_KEY, true);
	rb_memcached_error_check(rc);

	return rb_mc;
}

static memcached_return_t
iterate_server_function(const memcached_st *ptr, const memcached_instance_st * server, void *context)
{
	VALUE rb_server_list = (VALUE)context;
	VALUE rb_server = rb_funcall(rb_cServer, rb_intern("new"), 2, 
			rb_str_new2(memcached_server_name(server)),
			UINT2NUM(memcached_server_port(server)));

	rb_ary_push(rb_server_list, rb_server);
	return MEMCACHED_SUCCESS;
}

static VALUE
rb_connection_servers(VALUE self)
{
	memcached_server_fn callbacks[1] = { &iterate_server_function };
	memcached_st *mc;

	VALUE rb_result;

	UnwrapMemcached(self, mc);
	rb_result = rb_ary_new();

	memcached_server_cursor(mc, callbacks, (void *)rb_result, 1);
	return rb_result;
}

static VALUE
rb_connection_flush(VALUE self)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	rc = memcached_flush(mc, 0);
	rb_memcached_error_check(rc);

	return (rc == MEMCACHED_SUCCESS) ? Qtrue : Qfalse;
}

static VALUE
rb_connection_set(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);
	Check_Type(rb_value, T_STRING);
	Check_Type(rb_ttl, T_FIXNUM);
	Check_Type(rb_flags, T_FIXNUM);

	rc = memcached_set(mc,
		RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
		RSTRING_PTR(rb_value), RSTRING_LEN(rb_value),
		FIX2INT(rb_ttl), FIX2INT(rb_flags)
	);
			
	rb_memcached_return(rc);
}

static VALUE
rb_connection_get(VALUE self, VALUE rb_key)
{
	VALUE rb_val;

	memcached_st *mc;
	memcached_return_t rc;
	size_t ret_len;
	uint32_t ret_flags;
	char *ret;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);

	ret = memcached_get(mc,
		RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
		&ret_len, &ret_flags, &rc);

	if (ret == NULL) {
		rb_memcached_error_check(rc);
		return Qnil;
	}

	rb_val = rb_str_new(ret, ret_len);
	free(ret);

	/* TODO: return the flags too, maybe */
	return rb_val;
}

memcached_return_t
rb_connection__mget_callback(const memcached_st *mc, memcached_result_st *result, void *context)
{
	VALUE rb_result = (VALUE)context;
	VALUE rb_key = rb_str_new(
			memcached_result_key_value(result),
			memcached_result_key_length(result));
	VALUE rb_value = rb_str_new(
			memcached_result_value(result),
			memcached_result_length(result));

	rb_hash_aset(rb_result, rb_key, rb_value);
	return MEMCACHED_SUCCESS;
}

static VALUE
rb_connection_get_multi(VALUE self, VALUE rb_keys)
{
	memcached_execute_fn callbacks[1] = { &rb_connection__mget_callback };
	memcached_st *mc;
	memcached_return_t rc;

	long i, keys_len;
	const char **c_keys;
	size_t *c_lengths;

	VALUE rb_result;

	UnwrapMemcached(self, mc);

	Check_Type(rb_keys, T_ARRAY);
	keys_len = RARRAY_LEN(rb_keys);

	c_keys = alloca(keys_len * sizeof(const char *));
	c_lengths = alloca(keys_len * sizeof(size_t));

	for (i = 0; i < keys_len; i++) {
		VALUE rb_key = rb_ary_entry(rb_keys, i);
		Check_Type(rb_key, T_STRING);

		c_keys[i] = RSTRING_PTR(rb_key);
		c_lengths[i] = RSTRING_LEN(rb_key);
	}

	rc = memcached_mget(mc, c_keys, c_lengths, (size_t)keys_len);
	rb_memcached_error_check(rc);

	rb_result = rb_hash_new();
	rc = memcached_fetch_execute(mc, callbacks, (void *)rb_result, 1);
	rb_memcached_error_check(rc);

	return rb_result;
}

static VALUE
rb_connection_delete(VALUE self, VALUE rb_key)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);

	rc = memcached_delete(mc, RSTRING_PTR(rb_key), RSTRING_LEN(rb_key), (time_t)0);
	rb_memcached_return(rc);
}

static VALUE
rb_connection_add(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);
	Check_Type(rb_value, T_STRING);
	Check_Type(rb_ttl, T_FIXNUM);
	Check_Type(rb_flags, T_FIXNUM);

	rc = memcached_add(mc,
			RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
			RSTRING_PTR(rb_value), RSTRING_LEN(rb_value),
			FIX2INT(rb_ttl), FIX2INT(rb_flags)
	);

	rb_memcached_return(rc);
}

static VALUE
rb_connection_inc(VALUE self, VALUE rb_key, VALUE rb_value)
{
	memcached_st *mc;
	memcached_return_t rc;
	uint64_t result;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);
	Check_Type(rb_value, T_FIXNUM);

	rc = memcached_increment(mc,
		RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
		(uint32_t)FIX2ULONG(rb_value), &result
	);
	rb_memcached_error_check(rc);
	return ULL2NUM(result);
}

static VALUE
rb_connection_dec(VALUE self, VALUE rb_key, VALUE rb_value)
{
	memcached_st *mc;
	memcached_return_t rc;
	uint64_t result;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);
	Check_Type(rb_value, T_FIXNUM);

	rc = memcached_decrement(mc,
		RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
		(uint32_t)FIX2ULONG(rb_value), &result
	);
	rb_memcached_error_check(rc);
	return ULL2NUM(result);
}

static VALUE
rb_connection_exist(VALUE self, VALUE rb_key)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);

	rc = memcached_exist(mc, RSTRING_PTR(rb_key), RSTRING_LEN(rb_key));
	rb_memcached_error_check(rc);
	return (rc == MEMCACHED_SUCCESS) ? Qtrue : Qfalse;
}

static VALUE
rb_connection_replace(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);
	Check_Type(rb_value, T_STRING);
	Check_Type(rb_ttl, T_FIXNUM);
	Check_Type(rb_flags, T_FIXNUM);

	rc = memcached_replace(mc,
		RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
		RSTRING_PTR(rb_value), RSTRING_LEN(rb_value),
		FIX2INT(rb_ttl), FIX2INT(rb_flags)
	);
	rb_memcached_error_check(rc);
	return (rc == MEMCACHED_SUCCESS) ? Qtrue : Qfalse;
}

static VALUE
rb_connection_prepend(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);
	Check_Type(rb_value, T_STRING);
	Check_Type(rb_ttl, T_FIXNUM);
	Check_Type(rb_flags, T_FIXNUM);

	rc = memcached_prepend(mc,
		RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
		RSTRING_PTR(rb_value), RSTRING_LEN(rb_value),
		FIX2INT(rb_ttl), FIX2INT(rb_flags)
	);
	rb_memcached_return(rc);
}

static VALUE
rb_connection_append(VALUE self, VALUE rb_key, VALUE rb_value, VALUE rb_ttl, VALUE rb_flags)
{
	memcached_st *mc;
	memcached_return_t rc;

	UnwrapMemcached(self, mc);
	Check_Type(rb_key, T_STRING);
	Check_Type(rb_value, T_STRING);
	Check_Type(rb_ttl, T_FIXNUM);
	Check_Type(rb_flags, T_FIXNUM);

	rc = memcached_append(mc,
		RSTRING_PTR(rb_key), RSTRING_LEN(rb_key),
		RSTRING_PTR(rb_value), RSTRING_LEN(rb_value),
		FIX2INT(rb_ttl), FIX2INT(rb_flags)
	);
	rb_memcached_return(rc);
}

static VALUE
rb_connection_set_prefix(VALUE self, VALUE rb_prefix)
{
	memcached_st *mc;
	memcached_return_t rc;
	const char *prefix = NULL;

	UnwrapMemcached(self, mc);

	if (!NIL_P(rb_prefix)) {
		Check_Type(rb_prefix, T_STRING);
		prefix = StringValueCStr(rb_prefix);
	}

	rc = memcached_callback_set(mc, MEMCACHED_CALLBACK_NAMESPACE, prefix);
	rb_memcached_return(rc);
}

static VALUE
rb_connection_get_prefix(VALUE self)
{
	memcached_st *mc;
	memcached_return_t rc;
	const char *value;

	UnwrapMemcached(self, mc);

	value = memcached_callback_get(mc, MEMCACHED_CALLBACK_NAMESPACE, &rc);
	rb_memcached_error_check(rc);

	return value ? rb_str_new2(value) : Qnil;
}

void Init_memcached_rb(void)
{
	size_t i;

	rb_mMemcached = rb_define_module("Memcached");
	rb_cServer = rb_const_get(rb_mMemcached, rb_intern("Server"));
	rb_eMemcachedError = rb_define_class_under(rb_mMemcached, "Error", rb_eStandardError);

	for (i = 1; i < MEMCACHED_ERROR_COUNT; i++) {
		const char *klass = MEMCACHED_ERROR_NAMES[i];
		rb_eMemcachedErrors[i] = klass ? 
			rb_define_class_under(rb_mMemcached, klass, rb_eMemcachedError) : Qnil;
	}

	rb_cConnection = rb_define_class_under(rb_mMemcached, "Connection", rb_cObject);
	rb_define_singleton_method(rb_cConnection, "new", rb_connection_new, 1);

	rb_define_method(rb_cConnection, "servers", rb_connection_servers, 0);
	rb_define_method(rb_cConnection, "flush", rb_connection_flush, 0);
	rb_define_method(rb_cConnection, "set", rb_connection_set, 4);
	rb_define_method(rb_cConnection, "get", rb_connection_get, 1);
	rb_define_method(rb_cConnection, "get_multi", rb_connection_get_multi, 1);
	rb_define_method(rb_cConnection, "delete", rb_connection_delete, 1);
	rb_define_method(rb_cConnection, "add", rb_connection_add, 4);
	rb_define_method(rb_cConnection, "increment", rb_connection_inc, 2);
	rb_define_method(rb_cConnection, "decrement", rb_connection_dec, 2);
	rb_define_method(rb_cConnection, "exist", rb_connection_exist, 1);
	rb_define_method(rb_cConnection, "replace", rb_connection_replace, 4);
	rb_define_method(rb_cConnection, "prepend", rb_connection_prepend, 4);
	rb_define_method(rb_cConnection, "append", rb_connection_append, 4);
	rb_define_method(rb_cConnection, "get_behavior", rb_connection_get_behavior, 1);
	rb_define_method(rb_cConnection, "set_behavior", rb_connection_set_behavior, 2);
	rb_define_method(rb_cConnection, "set_prefix", rb_connection_set_prefix, 1);
	rb_define_method(rb_cConnection, "get_prefix", rb_connection_get_prefix, 0);

	id_tcp = rb_intern("tcp");
	id_socket = rb_intern("socket");

	Init_memcached_rb_behavior();
}
