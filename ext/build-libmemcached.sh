#!/bin/sh -e

cd libmemcached-src
./config/bootstrap
./configure --prefix=`pwd`/unused --libdir=`pwd`/.. --includedir=`pwd`/..
make && make install