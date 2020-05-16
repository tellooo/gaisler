#!/bin/bash

set -e

# get common RCC defines
. ./common.sh

if [ "$RCC_TOOLCHAIN" = "llvm" ]; then
    SELECTED_TARGET_CC=${TARGET}-clang
    SELECTED_TARGET_CXX=${TARGET}-clang++
else
    # note: uses defaults set by newlib when these are empty.
    SELECTED_TARGET_CC=${TARGET}-gcc
    SELECTED_TARGET_CXX=
fi

echo INST=$INST
echo SRCDIR=$SRCDIR
echo TARGET=$TARGET

export PATH=$INST/bin:$PATH

iconv_encodings="big5,cp775,cp850,cp852,cp855,\
cp866,euc_jp,euc_kr,euc_tw,iso_8859_1,iso_8859_10,iso_8859_11,\
iso_8859_13,iso_8859_14,iso_8859_15,iso_8859_2,iso_8859_3,\
iso_8859_4,iso_8859_5,iso_8859_6,iso_8859_7,iso_8859_8,iso_8859_9,\
iso_ir_111,koi8_r,koi8_ru,koi8_u,koi8_uni,ucs_2,ucs_2_internal,\
ucs_2be,ucs_2le,ucs_4,ucs_4_internal,ucs_4be,ucs_4le,us_ascii,\
utf_16,utf_16be,utf_16le,utf_8,win_1250,win_1251,win_1252,\
win_1253,win_1254,win_1255,win_1256,win_1257,win_1258"

# Configure newlib
rm -rf $SRCDIR/build-newlib
mkdir $SRCDIR/build-newlib
cd $SRCDIR/build-newlib
${SRCDIR}/newlib/configure \
      --prefix=$INST \
      --datarootdir=$INST \
      --with-pkgversion="$PKGVERSION" \
      --target=$TARGET \
      --with-cpu=leon3 \
      --disable-nls \
      --disable-fixed-point \
      --disable-decimal-float \
      --disable-shared \
      --enable-threads=rtems \
      --enable-lto \
      --with-gnu-as \
      --with-gnu-ld \
      --enable-newlib-io-c99-formats \
      --enable-newlib-iconv \
      --enable-newlib-iconv-encodings="$iconv_encodings" \
      --enable-version-specific-runtime-libs \
      --enable-libgomp \
      --without-included-gettext \
      CC=gcc \
      GCC_FOR_TARGET=$SELECTED_TARGET_CC \
      CC_FOR_TARGET=$SELECTED_TARGET_CC \
      CXX_FOR_TARGET=$SELECTED_TARGET_CXX \
      CFLAGS_FOR_TARGET="-g -O3"

# Build newlib
make -j32

# Install newlib
make install
