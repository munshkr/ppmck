#!/bin/sh

# compile ppmckc and nesasm for un*x.
# just run me, like this:
#    $ ./compile_unix.sh

EXEDIR=../bin

mkdir -p $EXEDIR
rm -f $EXEDIR/*.exe
rm -f nesasm/*.o
rm -f ppmckc/*.o

if cd nesasm; then
    make
    cd ..
fi

if cd ppmckc; then
    make EXESFX=
    cd ..
fi

