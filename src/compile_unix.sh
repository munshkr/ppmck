#!/bin/sh

# compile ppmckc and nesasm for un*x.
# just run me, like this:
#    $ chmod +x compile_unix.sh
#    $ ./compile_unix.sh

EXEDIR=../bin

mkdir -p $EXEDIR
rm -f $EXEDIR/*.exe
rm -f nesasm/*.o
rm -f ppmckc/*.o

if cd nesasm; then
    make -f Makefile.unx
    cd ..
fi

if cd ppmckc; then
    if which nkf > /dev/null; then
        nkf --overwrite -e -Lu *.c *.h
    else
        echo '#######WARNING#########'
        echo "$0: You need nkf (Network Kanji Filter) to compile ppmckc properly on un*x system."
        echo '#######################'
    fi
    make EXESFX=
    cd ..
fi

