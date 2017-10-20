#!/bin/sh -xe

cd $(dirname $0)/..

aclocal
autoconf
automake --add-missing --force-missing
./configure $features

make
