# mckc makefile for MinGW
.PHONY: all clean

RM = rm -f
CC = gcc
CFLAGS = -O2 -Wall# -mno-cygwin
CDEFS =
LDFLAGS=-Xlinker -s# -mno-cygwin

OBJS = datamake.o file.o mckc.o strings.o version.o
EXEC = ppmckc
EXESFX=   # use EXESFX=.exe for windows
EXEDIR=../../bin

TARGET = $(EXEDIR)/ppmckc$(EXESFX)

all: $(TARGET_E) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(CDEFS) -c $<

mckc.o: mckc.h
datamake.o: mckc.h

clean:
	$(RM) $(OBJS) ppmckc$(EXESFX)
