#!/bin/bash
#
# BCC2 build script
#

# Default parameter values
root=`pwd`
bdir=$root/build
logdir=$bdir/log
opt=/opt/bcc-2.0.0-user
TARGET=sparc-gaisler-elf
do_toolchain=0
do_gdb=0
do_libbcc=0

build_help () {
  echo "usage: $0 [options]"
  echo "Options:"
  echo "  --help                   Displays this information"
  echo "  --destination [path]     Destination path"
  echo "  --toolchain              Build toolchain"
  echo "  --gdb                    Build GDB"
#  echo "  --libbcc                 Build libbcc"
  echo ""
  echo "Example:"
  echo "$0 --destination /tmp/bcc-2.0-local --toolchain --gdb"
  echo ""
}

build_log () {
  fn=$1
  shift
  mkdir -p $logdir
  echo "> root=$root" >> $logdir/$fn.stdout
  echo "> bdir=$bdir" >> $logdir/$fn.stdout
  echo "> opt=$opt" >> $logdir/$fn.stdout
  echo "> $@" >> $logdir/$fn.stdout
  `bash -c "$*" 1>> $logdir/$fn.stdout 2>> $logdir/$fn.stderr` || \
    if [ $?!= 0 ]; then
      echo "FAIL: See $logdir/$fn.stderr"
      exit 1
    fi
}

build_log_clear () {
  fn=$1
  mkdir -p $logdir
  rm -f $logdir/$fn.stdout
  rm -f $logdir/$fn.stderr
}

build_toolchain () {
  build_log_clear binutils
  build_log_clear gcc1
  build_log_clear newlib
  build_log_clear gcc2

  echo "binutils"
  wdir=$bdir/binutils
  mkdir -p $wdir
  cd $wdir

  echo " - configure"
  build_log binutils "${root}/binutils/configure \
      --prefix=$opt \
      --target=$TARGET \
      --with-cpu= \
      --disable-shared \
      --disable-nls \
      --disable-gdb \
      --disable-sim \
      --with-sysroot=$opt/$TARGET \
      --with-build-sysroot=$bdir/$opt/$TARGET \
      --enable-plugins"

  echo " - build"
  build_log binutils "make -j32"

  echo " - install"
  build_log binutils "make install"


  echo "gcc1"
  wdir=$bdir/gcc
  mkdir -p $wdir
  cd $wdir

  echo " - configure"
  build_log gcc1 "CFLAGS_FOR_TARGET=\"-g -O3\" \
      ${root}/gcc/configure \
      --prefix=$opt \
      --target=$TARGET \
      --with-cpu= \
      --disable-nls \
      --disable-libmudflap \
      --disable-libssp \
      --enable-version-specific-runtime-libs \
      --disable-fixed-point \
      --disable-decimal-float \
      --enable-languages=c,c++ \
      --disable-shared \
      --disable-lto \
      --disable-libstdcxx-pch \
      --with-gnu-as \
      --with-gnu-ld \
      --disable-libgloss \
      --with-newlib \
      --with-sysroot=$opt/$TARGET \
      --with-build-sysroot=$bdir/$opt/$TARGET \
      --with-native-system-header-dir=/include"

  echo " - build"
  build_log gcc1 "make -j32 all-gcc"

  echo " - install"
  build_log gcc1 "make install-gcc"


  echo "newlib"
  wdir=$bdir/newlib
  mkdir -p $wdir
  cd $wdir

  echo " - configure"
  build_log newlib "CFLAGS_FOR_TARGET=\"-g -O3\" \
      ${root}/newlib/configure \
      --prefix=$opt \
      --target=$TARGET \
      --with-cpu= \
      --disable-nls \
      --disable-fixed-point \
      --disable-decimal-float \
      --disable-shared \
      --disable-lto \
      --with-gnu-as \
      --with-gnu-ld \
      --disable-libgloss \
      --enable-newlib-io-c99-formats \
      --enable-newlib-io-long-long \
      --disable-newlib-multithread \
      --enable-newlib-global-atexit \
      --enable-newlib-reent-small"

  echo " - build"
  build_log newlib "make -j32"

  echo " - install"
  build_log newlib "make install"


  echo "gcc2"
  wdir=$bdir/gcc
  cd $wdir

  echo " - build"
  build_log gcc2 "make -j32 all"

  echo " - install"
  build_log gcc2 "make install"
}

build_gdb () {
  build_log_clear gdb

  echo "gdb"
  wdir=$bdir/gdb
  mkdir -p $wdir
  cd $wdir

  echo " - configure"
  build_log gdb "${root}/gdb/configure \
      --prefix=$opt \
      --target=$TARGET \
      --with-cpu= \
      --disable-shared \
      --disable-nls \
      --disable-sim \
      --disable-werror \
      --with-sysroot=$opt/$TARGET \
      --with-build-sysroot=$bdir/$opt/$TARGET"

  echo " - build"
  build_log gdb "make -j32"

  echo " - install"
  build_log gdb "make install"
}

build_libbcc () {
  build_log_clear libbcc

  echo "libbcc"
  wdir=$bdir/bsp
  mkdir -p $wdir
  cd $wdir
  echo " - build"
  build_log libbcc "make -C ${root}/libbcc/ all BCC_PATH=${root}/libbcc BUILDDIR=$bdir/bsp DESTDIR=$opt/$TARGET/bsp"
}

if [ $# -eq 0 ]; then
  build_help
  exit
fi

# Parse commandline-switches
while [ $# -ne 0 ]; do
  if [ "$1" = "--help" ]; then
    build_help
    exit
  elif [ "$1" = "--destination" ]; then
    if [ -n "$2" ]; then
      opt=$2
      shift
    else
      build_help
      exit
    fi
  elif [ "$1" = "--toolchain" ]; then
    do_toolchain=1
  elif [ "$1" = "--gdb" ]; then
    do_gdb=1
  elif [ "$1" = "--libbcc" ]; then
    do_libbcc=1
  else
    echo "Unknown commandline switch: $1"
    build_help
    exit
  fi
  shift
done

PATH="$opt/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

echo "Destination: $opt"
echo "Build:       $bdir"
echo "Log:         $logdir"
echo "Begin date:  `date`"

if [ $do_toolchain -eq 1 ]; then
  build_toolchain
fi

if [ $do_gdb -eq 1 ]; then
  build_gdb
fi

if [ $do_libbcc -eq 1 ]; then
  build_libbcc
fi

echo "End date:    `date`"

