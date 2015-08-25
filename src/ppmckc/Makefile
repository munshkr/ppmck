# mckc makefile for MinGW

RM = rm -f
CC = gcc
CFLAGS = -O2 -Wall# -mno-cygwin
CDEFS = 
LDFLAGS=-Xlinker -s# -mno-cygwin

OBJS = datamake.o file.o mckc.o strings.o 
EXESFX=.exe
EXEDIR=../../bin


.PHONY: all clean

TARGET_E = $(EXEDIR)/ppmckc_e$(EXESFX)
TARGET = $(EXEDIR)/ppmckc$(EXESFX)

all: $(TARGET_E) $(TARGET)

$(TARGET_E): $(OBJS) version_e.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) version_e.o

$(TARGET): $(OBJS) version.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) version.o

version_e.o: version.c
	$(CC) $(CFLAGS) $(CDEFS) -DENGLISH -o$@ -c version.c

.c.o:
	$(CC) $(CFLAGS) $(CDEFS) -c $<

mckc.o: mckc.h
datamake.o: mckc.h

clean:
	$(RM) $(OBJS) version.o version_e.o ppmckc_e$(EXESFX) ppmckc$(EXESFX)
