#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRCDIR=$DIR/src
DESTDIR=$DIR/libs/mac
MACOSX_DEPLOYMENT_TARGET="10.6"

rm "$DESTDIR"/*.a

cd $SRCDIR
make clean
make CC="gcc -m64 -arch x86_64" clean all

if [ -f $SRCDIR/src/libluajit.a ]; then
    mv $SRCDIR/src/libluajit.a $DESTDIR/64-bit/libluajit.a
fi;

make clean
make CC="gcc -m32" clean all

if [ -f $SRCDIR/src/libluajit.a ]; then
mv $SRCDIR/src/libluajit.a $DESTDIR/32-bit/libluajit.a
fi;

make clean
