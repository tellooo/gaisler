
# sourced from build scripts

INST=`readlink -f $PWD/..`
SRCDIR=`readlink -f $PWD`
RCC_VERSION=1.3-rc7
TARGET=sparc-gaisler-rtems5
PKGVERSION="Custom built RCC $RCC_VERSION"

# Let user override, but default to toolchain build.
if [ -z "$RCC_TOOLCHAIN" ]; then
    RCC_TOOLCHAIN="gnu"
fi
