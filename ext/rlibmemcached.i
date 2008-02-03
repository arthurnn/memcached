%module rlibmemcached
%{
#include <libmemcached/memcached.h>
%}

%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_server_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_stat_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_string_st;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) memcached_result_st;

%include "typemaps.i"

//// Input maps
%apply unsigned short { uint8_t };
%apply unsigned int { uint16_t };
%apply unsigned long { uint32_t flags, uint32_t offset };

// For behavior's weird set interface
%typemap(in) (void *data) {
  int value = FIX2INT($input);
  if (value == 0 || value == 1) {
    $1 = (void *) value;
  } else {
    // Only pass by reference for :distribution and :hash 
    value = value - 2; 
    $1 = &value;
  }
};

// Array of strings map for multiget
%typemap(in) (char **keys, size_t *key_length, unsigned int number_of_keys) {
  int i;
  Check_Type($input, T_ARRAY);
  $3 = (unsigned int) RARRAY_LEN($input);
  $2 = (size_t *) malloc(($3+1)*sizeof(size_t));
  $1 = (char **) malloc(($3+1)*sizeof(char *)); 
  for(i = 0; i < $3; i ++) {
    $2[i] = strlen(StringValuePtr(RARRAY_PTR($input)[i]));
    $1[i] = StringValuePtr(RARRAY_PTR($input)[i]);
  }
}
%typemap(freearg) (char **keys, size_t *key_length, unsigned int number_of_keys) {
   free($1);
   free($2);
}

// Generic strings
%typemap(in) (char *str, size_t len) {
 $1 = STR2CSTR($input);
 $2 = (size_t) RSTRING($input)->len;
};

%apply (char *str, size_t len) {
  (char *key, size_t key_length), 
  (char *value, size_t value_length)
};

//// Output maps
%apply unsigned short *OUTPUT {memcached_return *error}
%apply unsigned int *OUTPUT {uint32_t *flags}
%apply size_t *OUTPUT {size_t *value_length}
%apply unsigned long long *OUTPUT {uint64_t *value}

// String
%typemap(in, numinputs=0) (char *key, size_t *key_length) {
  $1 = malloc(512*sizeof(char));
  $2 = malloc(sizeof(size_t));
}; 
%typemap(argout) (char *key, size_t *key_length) {
  if ($1 != NULL) {
    rb_ary_push($result, rb_str_new($1, *$2));
    free($1);
    free($2);
  }
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

%include "/opt/local/include/libmemcached/memcached.h"

// Manual wrappers

// Single get. SWIG likes to use SWIG_FromCharPtr instead of SWIG_FromCharPtrAndSize because 
// of the retval/argout split, so it truncates return values with \0 in them. 
VALUE memcached_get_rvalue(memcached_st *ptr, char *key, size_t key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_get_rvalue(memcached_st *ptr, char *key, size_t key_length, uint32_t *flags, memcached_return *error) {
  VALUE ret;  
  size_t *value_length = malloc(sizeof(size_t));
  char *value = memcached_get(ptr, key, key_length, value_length, flags, error);
  ret = rb_str_new(value, *value_length);
  free(value);
  free(value_length);
  return ret;
};
%}

// Multi get
VALUE memcached_fetch_rvalue(memcached_st *ptr, char *key, size_t *key_length, uint32_t *flags, memcached_return *error);
%{
VALUE memcached_fetch_rvalue(memcached_st *ptr, char *key, size_t *key_length, uint32_t *flags, memcached_return *error) {
  size_t *value_length = malloc(sizeof(size_t));
  VALUE result = rb_ary_new();
  
  char *value = memcached_fetch(ptr, key, key_length, value_length, flags, error);
  if (value == NULL) {
    rb_ary_push(result, Qnil);
  } else {
    VALUE ret = rb_str_new(value, *value_length);
    rb_ary_push(result, ret);
    free(value);
    free(value_length);
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
