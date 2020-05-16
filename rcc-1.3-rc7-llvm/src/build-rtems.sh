#!/bin/bash

set -e

# get common RCC defines
. ./common.sh

#BSPS_STD_UP="erc32 leon2std leon3std"
# leon3_flat disabled
RTEMSDIR=`ls -d1 rcc-* | head -1`
REL_VER=`echo $RTEMSDIR | sed 's/^rcc-//g'`

if [ "$RCC_TOOLCHAIN" = "llvm" ]; then
    SELECTED_TARGET_CC=${TARGET}-clang
    SELECTED_TARGET_CXX=${TARGET}-clang++
    BSPS_UP="leon3 leon3_sf gr712rc gr740 ut700"
    BSPS_SMP="leon3_smp gr712rc_smp gr740_smp"
else
    # note: uses defaults set by newlib when these are empty.
    SELECTED_TARGET_CC=${TARGET}-gcc
    SELECTED_TARGET_CXX=
    BSPS_UP="leon3 leon3_sf gr712rc gr740 ut699 ut700"
    BSPS_SMP="leon3_smp gr712rc_smp gr740_smp"
fi

echo BSPS_UP: $BSPS_UP
echo BSPS_SMP: $BSPS_SMP
echo RTEMSDIR: $RTEMSDIR
echo TARGET: $TARGET
echo INST: $INST
echo COMPILER: $SELECTED_TARGET_CC

export PATH=$INST/bin:$PATH
automake --version
autoconf --version

cd $RTEMSDIR
./bootstrap -c
./bootstrap -H
./bootstrap
cd ..

rm -rf build-up build-smp
mkdir build-up build-smp

# Build Single-core BSPs first
cd build-up
echo $SRCDIR/$RTEMSDIR/configure --prefix=$INST \
	--target=$TARGET --disable-itron --disable-tests --enable-cxx \
        --disable-docs --enable-drvmgr --enable-sparc-fpu \
	--enable-rtemsbsp="$BSPS_UP" \
        CC_FOR_TARGET=$SELECTED_TARGET_CC \
        CC_FOR_HOST=$SELECTED_TARGET_CC \
        CXX_FOR_TARGET=$SELECTED_TARGET_CXX \
        CXX_FOR_HOST=$SELECTED_TARGET_CXX \
	RCC_VERSION=\"$REL_VER\"

$SRCDIR/$RTEMSDIR/configure --prefix=$INST \
	--target=$TARGET --disable-itron --disable-tests --enable-cxx \
        --disable-docs --enable-drvmgr --enable-sparc-fpu \
	--enable-rtemsbsp="$BSPS_UP" \
        CC_FOR_TARGET=$SELECTED_TARGET_CC \
        CC_FOR_HOST=$SELECTED_TARGET_CC \
        CXX_FOR_TARGET=$SELECTED_TARGET_CXX \
        CXX_FOR_HOST=$SELECTED_TARGET_CXX \
	RCC_VERSION=\"$REL_VER\"

make all -j8
make install -j8


# Build multi-core SMP BSPs first
cd ../build-smp
echo $SRCDIR/$RTEMSDIR/configure --prefix=$INST \
	--target=$TARGET --disable-itron --disable-tests --enable-cxx \
        --disable-docs --enable-drvmgr --enable-sparc-fpu \
	--enable-smp \
	--enable-rtemsbsp="$BSPS_SMP" \
        CC_FOR_TARGET=$SELECTED_TARGET_CC \
        CC_FOR_HOST=$SELECTED_TARGET_CC \
        CXX_FOR_TARGET=$SELECTED_TARGET_CXX \
        CXX_FOR_HOST=$SELECTED_TARGET_CXX \
	RCC_VERSION=\"$REL_VER\"

$SRCDIR/$RTEMSDIR/configure --prefix=$INST \
	--target=$TARGET --disable-itron --disable-tests --enable-cxx \
        --disable-docs --enable-drvmgr --enable-sparc-fpu \
	--enable-smp \
	--enable-rtemsbsp="$BSPS_SMP" \
        CC_FOR_TARGET=$SELECTED_TARGET_CC \
        CC_FOR_HOST=$SELECTED_TARGET_CC \
        CXX_FOR_TARGET=$SELECTED_TARGET_CXX \
        CXX_FOR_HOST=$SELECTED_TARGET_CXX \
	RCC_VERSION=\"$REL_VER\"

make all -j8
make install -j8
