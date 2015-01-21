#!/bin/bash

PLATFORM_PREFIX=/home/dmitry/android-ext/
NDK_PATH=/home/dmitry/android-ndk-r10d/
NDK_PLATFORM=android-9

mkdir $PLATFORM_PREFIX
#mkdir $PLATFORM_PREFIX/arm
mkdir $PLATFORM_PREFIX/x86
#$NDK_PATH/build/tools/make-standalone-toolchain.sh  --platform=$NDK_PLATFORM --install-dir=$PLATFORM_PREFIX/arm --toolchain=arm-linux-androideabi-4.9 --system=linux-x86_64 --arch=arm
#$NDK_PATH/build/tools/make-standalone-toolchain.sh  --platform=$NDK_PLATFORM --install-dir=$PLATFORM_PREFIX/x86 --toolchain=x86-4.9 --system=linux-x86_64 --arch=x86

FILENAME=freetype-2.5.5
EXTENSION=tar.bz2
wget -c http://download.savannah.gnu.org/releases/freetype/$FILENAME.$EXTENSION -O ../$FILENAME.$EXTENSION
tar -jxf ../$FILENAME.$EXTENSION

cd $FILENAME

#export LDFLAGS="-Wl,--fix-cortex-a8"
#export CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfp -marm -fsigned-char -std=c99"
#export LIBS="-lc"

NDK_ROOT=/home/dmitry/android-ndk-r10d
NDK=$NDK_ROOT
NDKABI=8
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/${host_os}-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
NDKABI=14
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/${host_os}-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
NDKARCH="-march=armv7-a -mfloat-abi=softfp -Wl,--fix-cortex-a8"

#export LDFLAGS="-Wl,--fix-cortex-a8"

#./configure --sysroot $PLATFORM_PREFIX/arm \
 #           LDFLAGS='-Wl,--fix-cortex-a8' \
  #          CFLAGS='-march=armv7-a -mfloat-abi=softfp -mfpu=vfp -fsigned-char' \
   #         --host=arm-linux-androideabi \
    #        --prefix=/home/dmitry/android-ext/arm \
     #       --without-zlib \
      #      --with-png=no

export PATH="$PLATFORM_PREFIX/bin:$PATH"
which gcc
#export LDFLAGS="-Wl,--fix-cortex-a8"

#./configure --without-zlib --whiwith-png=no --host=arm-linux-androideabi
#make clean
#make HOST_CC="gcc -marm" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF $NDKARCH"
#make HOST_CC="gcc -m32"  LDFLAGS='-Wl,--fix-cortex-a8' CFLAGS='--sysroot $PLATFORM_PREFIX/arm -march=armv7-a -mfloat-abi=softfp -mfpu=vfp -marm -fsigned-char'
#make LDFLAGS='-Wl,--fix-cortex-a8' CFLAGS="$NDKF $NDKARCH"
#make install

unset LDFLAGS
