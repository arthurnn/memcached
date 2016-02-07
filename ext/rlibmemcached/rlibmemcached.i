%module rlibmemcached
%{
#include <libmemcached/visibility.h>
#include <libmemcached/memcached.h>
#include <libmemcached/memcached_exist.h>
%}

%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_server_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_stat_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_string_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_result_st;

%include "typemaps.i"
%include "libmemcached/visibility.h"

//// Memory management

// Register libmemcached's struct free function to prevent memory leaks
%freefunc memcached_st "memcached_free";
%freefunc memcached_stat_st "memcached_stat_free";
%freefunc memcached_server_st "memcached_server_free";

// Register which functions generate new objects
%newobject memcached_create;
%newobject memcached_clone;
%newobject memcached_stat_get_value;
// %newobject memcached_stat;

// %trackobjects; // Doesn't fix any interesting leaks

//// Input maps

%apply unsigned short { uint8_t };
%apply unsigned int { uint16_t };
%apply unsigned int { uint32_t server_failure_counter };
%apply unsigned int { uint32_t user_spec_len };
%apply unsigned long { uint32_t flags, uint32_t offset, uint32_t weight, time_t expiration };
%apply unsigned long long { uint64_t data, uint64_t cas };

// Array of strings map for multiget
%typemap(in) (const char **keys, size_t *key_length, size_t number_of_keys) {
  unsigned int i;
  VALUE str;
  Check_Type($input, T_ARRAY);
  $3 = (unsigned int) RARRAY_LEN($input);
  $2 = (size_t *) malloc(($3+1)*sizeof(size_t));
  $1 = (char **) malloc(($3+1)*sizeof(char *));
  for(i = 0; i < $3; i ++) {
    str = rb_string_value(&RARRAY_PTR($input)[i]);
    $1[i] = RSTRING_PTR(str);
    $2[i] = RSTRING_LEN(str);
  }
}

%typemap(freearg) (const char **keys, size_t *key_length, size_t number_of_keys) {
   free($1);
   free($2);
}

// Generic strings
%typemap(in) (const char *str, size_t len) {
  VALUE str;
  str = rb_string_value(&$input);
  $1 = RSTRING_PTR(str);
  $2 = RSTRING_LEN(str);
};

// Void type strings without lengths for prefix_key callback
%typemap(in) (void *data) {
  VALUE str;
  str = rb_string_value(&$input);
  if (RSTRING_LEN(str) == 0) {
    $1 = NULL;
  } else {
    $1 = RSTRING_PTR(str);
  }
};

%apply (const char *str, size_t len) {
  (const char *namespace, size_t namespace_length),
  (const char *key, size_t key_length),
  (const char *value, size_t value_length)
};

// Key strings with same master key
// This will have to go if people actually want to set the master key separately
%typemap(in) (const char *master_key, size_t master_key_length, const char *key, size_t key_length) {
  VALUE str;
  str = rb_string_value(&$input);
  $3 = $1 = RSTRING_PTR(str);
  $4 = $2 = RSTRING_LEN(str);
};

//// Output maps

%apply unsigned short *OUTPUT {memcached_return *error}
%apply unsigned int *OUTPUT {uint32_t *flags}
%apply size_t *OUTPUT {size_t *value_length}
%apply unsigned long long *OUTPUT {uint64_t *value}

// Uint64
%typemap(out) (uint64_t) {
  $result = ULL2NUM($1);
};

// Uint32
%typemap(out) (uint32_t) {
 $result = UINT2NUM($1);
};

// String for memcached_fetch
%typemap(in, numinputs=0) (char *key, size_t *key_length) {
  char string[256];
  size_t length = 0;
  $1 = string;
  $2 = &length;
};

// Strings with lengths
%typemap(argout) (char *key, size_t *key_length) {
  rb_ary_push($result, rb_str_new($1, *$2));
}

// Array of strings
// Only used by memcached_stat_get_keys() and not performance-critical
%typemap(out) (char **) {
  int i;
  VALUE ary = rb_ary_new();
  $result = rb_ary_new();

  for(i=0; $1[i] != NULL; i++) {
    rb_ary_store(ary, i, rb_str_new2($1[i]));
  }
  rb_ary_push($result, ary);
  free($1);
};

//// SWIG includes, for functions, constants, and structs

%include "libmemcached/visibility.h"
%include "libmemcached/memcached.h"
%include "libmemcached/memcached_constants.h"
%include "libmemcached/memcached_get.h"
%include "libmemcached/memcached_storage.h"
%include "libmemcached/memcached_result.h"
%include "libmemcached/memcached_server.h"
%include "libmemcached/memcached_sasl.h"
%include "libmemcached/memcached_touch.h"
%include "libmemcached/memcached_exist.h"

//// Custom C functions

VALUE rb_str_new_by_ref(char *ptr, long len);
%{
VALUE rb_str_new_by_ref(char *ptr, long len)
{
#ifdef OBJSETUP
    NEWOBJ(str, struct RString);
    OBJSETUP(str, rb_cString, T_STRING);
    #ifdef RSTRING_NOEMBED
        /* Ruby 1.9 */
        str->as.heap.ptr = ptr;
        str->as.heap.len = len;
        str->as.heap.aux.capa = len + 1;
        // Set STR_NOEMBED
        FL_SET(str, FL_USER1);
    #else
        /* Ruby 1.8 */
        str->ptr = ptr;
        str->len = len;
        str->aux.capa = 0;
    #endif
#else
    /* Rubinius, JRuby */
    VALUE str = rb_str_new(ptr, len);
    free(ptr);
#endif
    return (VALUE)str;
}
%}

//// Manual wrappers

// Single get
VALUE memcached_get_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_get_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t *flags, memcached_return *error) {
  size_t value_length = 0;
  char *value = memcached_get(ptr, key, key_length, &value_length, flags, error);
  return rb_str_new_by_ref(value, value_length);
};
%}

VALUE memcached_get_len_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t user_spec_len, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_get_len_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t user_spec_len, uint32_t *flags, memcached_return *error) {
  size_t value_length = 0;
  char *value = memcached_get_len(ptr, key, key_length, user_spec_len, &value_length, flags, error);
  return rb_str_new_by_ref(value, value_length);
};
%}

VALUE memcached_get_from_last_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_get_from_last_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t *flags, memcached_return *error) {
  size_t value_length = 0;
  char *value = memcached_get_from_last(ptr, key, key_length, &value_length, flags, error);
  return rb_str_new_by_ref(value, value_length);
};
%}

// Multi get
VALUE memcached_fetch_rvalue(memcached_st *ptr, char *key, size_t *key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_fetch_rvalue(memcached_st *ptr, char *key, size_t *key_length, uint32_t *flags, memcached_return *error) {
  size_t value_length = 0;
  VALUE ary = rb_ary_new();
  *key_length = 0;
  if (error) *error = MEMCACHED_TIMEOUT; // timeouts leave error uninitialized
  char *value = memcached_fetch(ptr, key, key_length, &value_length, flags, error);
  VALUE str = rb_str_new_by_ref(value, value_length);
  rb_ary_push(ary, str);
  return ary;
};
%}

// Ruby isn't aware that the pointer is an array... there is probably a better way to do this
memcached_server_st *memcached_select_server_at(memcached_st *in_ptr, int index);
%{
memcached_server_st *memcached_select_server_at(memcached_st *in_ptr, int index) {
  return &(in_ptr->hosts[index]);
};
%}

// Same, but for stats
memcached_stat_st *memcached_select_stat_at(memcached_st *in_ptr, memcached_stat_st *stat_ptr, int index);
%{
memcached_stat_st *memcached_select_stat_at(memcached_st *in_ptr, memcached_stat_st *stat_ptr, int index) {
  return &(stat_ptr[index]);
};
%}

// Wrap only hash function
// Uint32
VALUE memcached_generate_hash_rvalue(const char *key, size_t key_length, memcached_hash hash_algorithm);
%{
VALUE memcached_generate_hash_rvalue(const char *key, size_t key_length,memcached_hash hash_algorithm) {
  return UINT2NUM(memcached_generate_hash_value(key, key_length, hash_algorithm));
};
%}

// Initialization for SASL
%init %{
  if (sasl_client_init(NULL) != SASL_OK) {
    fprintf(stderr, "Failed to initialized SASL.\n");
  }
%}
