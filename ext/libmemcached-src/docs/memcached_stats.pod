=head1 NAME

memcached_stat memcached_stat_servername memcached_stat_get_value memcached_stat_get_keys

=head1 LIBRARY

C Client Library for memcached (libmemcached, -lmemcached)

=head1 SYNOPSIS

  #include <memcached.h>

  memcached_stat_st *memcached_stat (memcached_st *ptr,
                                     char *args,
                                     memcached_return *error);

  memcached_return memcached_stat_servername (memcached_stat_st *stat,
                                              char *args, 
                                              char *hostname,
                                              unsigned int port);

  char *memcached_stat_get_value (memcached_st *ptr,
                                  memcached_stat_st *stat, 
                                  char *key,
                                  memcached_return *error);

  char ** memcached_stat_get_keys (memcached_st *ptr,
                                   memcached_stat_st *stat, 
                                   memcached_return *error);

=head1 DESCRIPTION

libmemcached(3) has the ability to query a memcached server (or collection
of servers) for their current state. Queries to find state return a
C<memcached_stat_st> structure. You are responsible for freeing this structure.
While it is possible to access the structure directly it is not advisable.
<memcached_stat_get_value() has been provided to query the structure.

memcached_stat() fetches an array of C<memcached_stat_st> structures containing
the state of all available memcached servers. The return value must be freed
by the calling application.

memcached_stat_servername() can be used standalone without a C<memcached_st> to
obtain the state of a particular server.  "args" is used to define a
particular state object (a list of these are not provided for by either
the memcached_stat_get_keys() call nor are they defined in the memcached
protocol). You must specify the hostname and port of the server you want to
obtain information on.

memcached_stat_get_value() returns the value of a particular state key. You
specify the key you wish to obtain.  The key must be null terminated.

memcached_stat_get_keys() returns a list of keys that the server has state
objects on. You are responsible for freeing this list.

A command line tool, memstat(1), is provided so that you do not have to write
an application to do this.

=head1 RETURN

Varies, see particular functions.

Any method returning a C<memcached_stat_st> expects you to free the
memory allocated for it.

=head1 HOME

To find out more information please check:
L<http://tangent.org/552/libmemcached.html>

=head1 AUTHOR

Brian Aker, E<lt>brian@tangent.orgE<gt>

=head1 SEE ALSO

memcached(1) libmemcached(3) memcached_strerror(3)

=cut

