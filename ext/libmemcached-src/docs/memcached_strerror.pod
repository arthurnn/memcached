=head1 NAME

memcached_strerror

=head1 LIBRARY

C Client Library for memcached (libmemcached, -lmemcached)

=head1 SYNOPSIS

  #include <memcached.h>

  char *memcached_strerror (memcached_st *ptr,
                            memcached_return rc);

=head1 DESCRIPTION

memcached_strerror() takes a C<memcached_return> value and returns a string
describing the error.

This string must not be modified by the application.

C<memcached_return> values are returned from nearly all libmemcached(3) functions.

C<memcached_return> values are of an enum type so that you can set up responses
with switch/case and know that you are capturing all possible return values.

=head1 RETURN

memcached_strerror() returns a string describing a C<memcached_return> value.

=head1 HOME

To find out more information please check:
L<http://tangent.org/552/libmemcached.html>

=head1 AUTHOR

Brian Aker, E<lt>brian@tangent.orgE<gt>

=head1 SEE ALSO

memcached(1) libmemcached(3)

=cut

