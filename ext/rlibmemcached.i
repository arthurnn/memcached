%module rlibmemcached
%{
#include <libmemcached/visibility.h>
#include <libmemcached/memcached.h>
%}

%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_server_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_stat_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_string_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_result_st;

%include "typemaps.i"
%include "libmemcached/visibility.h"

// Register libmemcached's struct free function to prevent memory leaks
%freefunc memcached_st "memcached_free";
%freefunc memcached_server_st "memcached_server_free";

// %trackobjects; // Doesn't fix any interesting leaks

//// Input maps

%apply unsigned short { uint8_t };
%apply unsigned int { uint16_t };
%apply unsigned long { uint32_t flags, uint32_t offset, uint32_t weight };
%apply unsigned long long { uint64_t data, uint64_t cas };

// Array of strings map for multiget
%typemap(in) (const char **keys, size_t *key_length, size_t number_of_keys) {
  int i;
  Check_Type($input, T_ARRAY);
  $3 = (unsigned int) RARRAY_LEN($input);
  $2 = (size_t *) malloc(($3+1)*sizeof(size_t));
  $1 = (char **) malloc(($3+1)*sizeof(char *)); 
  for(i = 0; i < $3; i ++) {
    Check_Type(RARRAY_PTR($input)[i], T_STRING);
    $2[i] = RSTRING_LEN(RARRAY_PTR($input)[i]);
    $1[i] = StringValuePtr(RARRAY_PTR($input)[i]);
  }
}
%typemap(in) (char **keys, size_t *key_length, unsigned int number_of_keys) {
  int i;
  Check_Type($input, T_ARRAY);
  $3 = (unsigned int) RARRAY_LEN($input);
  $2 = (size_t *) malloc(($3+1)*sizeof(size_t));
  $1 = (char **) malloc(($3+1)*sizeof(char *)); 
  for(i = 0; i < $3; i ++) {
    Check_Type(RARRAY_PTR($input)[i], T_STRING);
    $2[i] = RSTRING_LEN(RARRAY_PTR($input)[i]);
    $1[i] = StringValuePtr(RARRAY_PTR($input)[i]);
  }
}
%typemap(freearg) (char **keys, size_t *key_length, size_t number_of_keys) {
   free($1);
   free($2);
}

// Generic strings
%typemap(in) (const char *str, size_t len) {
 $1 = STR2CSTR($input);
 $2 = (size_t) RSTRING_LEN($input);
};

// Void type strings without lengths for prefix_key callback
%typemap(in) (void *data) {
  if ( (size_t) RSTRING_LEN($input) == 0) {
    $1 = NULL;
  } else {
    $1 = STR2CSTR($input);
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
 $3 = $1 = STR2CSTR($input);
 $4 = $2 = (size_t) RSTRING_LEN($input);
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

// Void type strings without lengths for prefix_key callback
// Currently not used by the gem
%typemap(out) (void *) {
 $result = rb_str_new2($1);
};

// String for memcached_fetch
%typemap(in, numinputs=0) (char *key, size_t *key_length) {
  char string[256];
  size_t length = 0;
  $1 = string;
  $2 = &length;
}; 
%typemap(argout) (char *key, size_t *key_length) {
  // Pushes an empty string when *key_length == 0
  rb_ary_push($result, rb_str_new($1, *$2)); 
}

// Array of strings

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

//// Custom C functions

//// Manual wrappers

// Single get. SWIG likes to use SWIG_FromCharPtr instead of SWIG_FromCharPtrAndSize because 
// of the retval/argout split, so it truncates return values with \0 in them. 
VALUE memcached_get_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_get_rvalue(memcached_st *ptr, const char *key, size_t key_length, uint32_t *flags, memcached_return *error) {
  VALUE ret;  
  size_t value_length;
  char *value = memcached_get(ptr, key, key_length, &value_length, flags, error);
  ret = rb_str_new(value, value_length);
  free(value);
  return ret;
};
%}

// Multi get
VALUE memcached_fetch_rvalue(memcached_st *ptr, char *key, size_t *key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_fetch_rvalue(memcached_st *ptr, char *key, size_t *key_length, uint32_t *flags, memcached_return *error) {
  size_t value_length;
  VALUE result = rb_ary_new();
  
  char *value = memcached_fetch(ptr, key, key_length, &value_length, flags, error);
  if (value == NULL) {
    rb_ary_push(result, Qnil);
  } else {
    VALUE ret = rb_str_new(value, value_length);
    rb_ary_push(result, ret);
    free(value);
  }
  return result;
};
%}

// We need to wrap this so it doesn't leak memory. SWIG doesn't want to automatically free. We could
// maybe use a 'ret' %typemap, but this is ok.
VALUE memcached_stat_get_rvalue(memcached_st *ptr, memcached_stat_st *stat, char *key, memcached_return *error);
%{
VALUE memcached_stat_get_rvalue(memcached_st *ptr, memcached_stat_st *stat, char *key, memcached_return *error) {
  char *str;
  VALUE ret;
  str = memcached_stat_get_value(ptr, stat, key, error);
  ret = rb_str_new2(str);
  free(str);
  return ret;
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

