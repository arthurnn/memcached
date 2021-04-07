# memcached

An interface to the libmemcached C client.
[![Build Status](https://github.com/arthurnn/memcached/workflows/CI/badge.svg?branch=1-0-stable)](https://github.com/arthurnn/memcached/actions?query=branch%3A1-0-stable)

## License

Copyright 2009-2013 Cloudburst, LLC. Licensed under the AFL 3. See the
included LICENSE file. Portions copyright 2007-2009 TangentOrg, Brian Aker,
licensed under the BSD license, and used with permission.

## Features

*   clean API
*   robust access to all memcached features
*   SASL support for the binary protocol
*   multiple hashing modes, including consistent hashing
*   ludicrous speed, including optional pipelined IO with no_reply


The **memcached** library wraps the pure-C libmemcached client via SWIG.

## Installation

You need Ruby 1.8.7 or Ruby 1.9.2. Other versions may work, but are not
guaranteed. You also need the `libsasl2-dev` and `gettext` libraries, which
should be provided through your system's package manager.

Install the gem:
    sudo gem install memcached --no-rdoc --no-ri

## Usage

Start a local networked memcached server:
    $ memcached -p 11211 &

Now, in Ruby, require the library and instantiate a Memcached object at a
global level:

    require 'memcached'
    $cache = Memcached.new("localhost:11211")

Now you can set things and get things:

    value = 'hello'
    $cache.set 'test', value
    $cache.get 'test' #=> "hello"

You can set with an expiration timeout:

    value = 'hello'
    $cache.set 'test', value, 1
    sleep(2)
    $cache.get 'test' #=> raises Memcached::NotFound

You can get multiple values at once:

    value = 'hello'
    $cache.set 'test', value
    $cache.set 'test2', value
    $cache.get ['test', 'test2', 'missing']
      #=> {"test" => "hello", "test2" => "hello"}

You can set a counter and increment it. Note that you must initialize it with
an integer, encoded as an unmarshalled ASCII string:

    start = 1
    $cache.set 'counter', start.to_s, 0, false
    $cache.increment 'counter' #=> 2
    $cache.increment 'counter' #=> 3
    $cache.get('counter', false).to_i #=> 3

You can get some server stats:

    $cache.stats #=> {..., :bytes_written=>[62], :version=>["1.2.4"] ...}

Note that the API is not the same as that of **Ruby-MemCache** or
**memcache-client**. In particular, `nil` is a valid record value.
Memcached#get does not return `nil` on failure, rather it raises
**Memcached::NotFound**. This is consistent with the behavior of memcached
itself. For example:

    $cache.set 'test', nil
    $cache.get 'test' #=> nil
    $cache.delete 'test'
    $cache.get 'test' #=> raises Memcached::NotFound

## Rails 3 and 4

Use [memcached_store gem](https://github.com/Shopify/memcached_store) to
integrate ActiveSupport cache store and memcached gem

## Pipelining

Pipelining updates is extremely effective in **memcached**, leading to more
than 25x write throughput than the default settings. Use the following options
to enable it:

    :no_block => true,
    :buffer_requests => true,
    :noreply => true,
    :binary_protocol => false

Currently #append, #prepend, #set, and #delete are pipelined. Note that when
you perform a read, all pending writes are flushed to the servers.

## Threading

**memcached** is threadsafe, but each thread requires its own Memcached
instance. Create a global Memcached, and then call Memcached#clone each time
you spawn a thread.

    thread = Thread.new do
      cache = $cache.clone
      # Perform operations on cache, not $cache
      cache.set 'example', 1
      cache.get 'example'
    end

    # Join the thread so that exceptions don't get lost
    thread.join

## Legacy applications

There is a compatibility wrapper for legacy applications called
Memcached::Rails.

## Benchmarks

**memcached**, correctly configured, is at least twice as fast as
**memcache-client** and **dalli**. See link:BENCHMARKS for details.

## Reporting problems

The support forum is [here](http://github.com/evan/memcached/issues).

Patches and contributions are very welcome. Please note that contributors are
required to assign copyright for their additions to Cloudburst, LLC.

## Further resources

*   [Memcached wiki](https://github.com/memcached/memcached/wiki)
*   [Libmemcached homepage](libmemcached.org)

