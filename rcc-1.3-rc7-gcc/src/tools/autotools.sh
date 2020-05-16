#!/bin/bash

if ! which readlink > /dev/null; then
	echo readlink utility is missing, please install it;
fi

set -e

PREFIX=`readlink -f $PWD/../../`
#PREFIX=/opt/rcc-1.3-rc1

# For RTEMS
AUTOCONF_VER=2.69
AUTOMAKE_VER=1.12.6

rm -rf	automake-$AUTOMAKE_VER \
	autoconf-$AUTOCONF_VER

tar -xf automake-$AUTOMAKE_VER.tar.xz
tar -xf autoconf-$AUTOCONF_VER.tar.xz

mkdir	automake-$AUTOMAKE_VER/build \
	autoconf-$AUTOCONF_VER/build

# set up PATH
export PATH=$PREFIX/bin:$PATH
export LD_LIBRARY_PATH=$PREFIX/lib:$LD_LIBRARY_PATH

# Autoconf
cd autoconf-$AUTOCONF_VER/build
../configure --prefix=$PREFIX
make all
make install
cd ../..

# Automake
patch automake-1.12.6/automake.in automake-1.12.6-bugzilla.redhat.com-1239379.diff
cd automake-$AUTOMAKE_VER/build
../configure --prefix=$PREFIX
make all
make install
cd ../..
