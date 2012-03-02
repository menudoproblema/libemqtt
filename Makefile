SRC=src
INC=include
LIB=.

CC=gcc
CCFLAGS=-I$(INC) -Wall -O
LDFLAGS=-L$(LIB) -lemqtt
AR=ar

all: pub sub

pub: libemqtt.a pub.o
	$(CC) pub.o -o pub $(LDFLAGS)

pub.o: pub.c $(INC)/libemqtt.h
	$(CC) $(CCFLAGS) -c pub.c -o pub.o

sub: libemqtt.a sub.o
	$(CC) sub.o -o sub $(LDFLAGS)

sub.o: sub.c $(INC)/libemqtt.h
	$(CC) $(CCFLAGS) -c sub.c -o sub.o

libemqtt.a: libemqtt.o
	$(AR) rcs libemqtt.a libemqtt.o

libemqtt.o: $(SRC)/libemqtt.c $(INC)/libemqtt.h
	$(CC) $(CCFLAGS) -c $(SRC)/libemqtt.c -o libemqtt.o

clean:
	rm -f libemqtt.o libemqtt.a pub.o sub.o

dist-clean: clean
	rm -f pub sub
