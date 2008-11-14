=head1 NAME

memcached_verbosity

=head1 LIBRARY

C Client Library for memcached (libmemcached, -lmemcached)

=head1 SYNOPSIS

  #include <memcached.h>

  memcached_return memcached_verbosity (memcached_st *ptr,
                                        unsigned int verbosity);

=head1 DESCRIPTION

memcached_verbosity() modifies the "verbosity" of the
memcached(1) servers referenced in the C<memcached_st> parameter.

=head1 RETURN

A value of type C<memcached_return> is returned
On success that value will be C<MEMCACHED_SUCCESS>.
Use memcached_strerror() to translate this value to a printable string.

=head1 HOME

To find out more information please check:
L<http://tangent.org/552/libmemcached.html>

=head1 AUTHOR

Brian Aker, E<lt>brian@tangent.orgE<gt>

=head1 SEE ALSO

memcached(1) libmemcached(3) memcached_strerror(3)

=cut

