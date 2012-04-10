SRC=src
INC=include
LIB=.
CLIENT=client

CC=gcc
CCFLAGS=-I$(INC) -Wall -O
LDFLAGS=-L$(LIB) -lemqtt
AR=ar

PYTHON_VER=2.7
PYTHON_SRC=src/python
PYTHON_CCFLAGS=-I$(INC) -I$(PYTHON_SRC) -I/usr/include/python$(PYTHON_VER) -Wall -O -fPIC
PYTHON_LDFLAGS=-shared



c: $(CLIENT)/pub $(CLIENT)/sub

$(CLIENT)/pub: libemqtt.a pub.o
	$(CC) pub.o -o $(CLIENT)/pub $(LDFLAGS)

pub.o: $(CLIENT)/pub.c $(INC)/libemqtt.h
	$(CC) $(CCFLAGS) -c $(CLIENT)/pub.c -o pub.o

$(CLIENT)/sub: libemqtt.a sub.o
	$(CC) sub.o -o $(CLIENT)/sub $(LDFLAGS)

sub.o: $(CLIENT)/sub.c $(INC)/libemqtt.h
	$(CC) $(CCFLAGS) -c $(CLIENT)/sub.c -o sub.o





python: python-libemqtt.o python-mqtt.o python-mqtt_packet.o libemqtt-python.o
	$(CC) $(PYTHON_LDFLAGS)  python-libemqtt.o python-mqtt.o python-mqtt_packet.o libemqtt-python.o -o $(CLIENT)/libemqtt.so

python-libemqtt.o: $(PYTHON_SRC)/python-libemqtt.c $(INC)/libemqtt.h $(PYTHON_SRC)/python-mqtt.h $(PYTHON_SRC)/python-mqtt_packet.h
	$(CC) $(PYTHON_CCFLAGS) -c $(PYTHON_SRC)/python-libemqtt.c -o python-libemqtt.o

python-mqtt.o: $(PYTHON_SRC)/python-mqtt.c $(INC)/libemqtt.h $(PYTHON_SRC)/python-mqtt.h
	$(CC) $(PYTHON_CCFLAGS) -c $(PYTHON_SRC)/python-mqtt.c -o python-mqtt.o

python-mqtt_packet.o: $(PYTHON_SRC)/python-mqtt_packet.c $(INC)/libemqtt.h $(PYTHON_SRC)/python-mqtt_packet.h
	$(CC) $(PYTHON_CCFLAGS) -c $(PYTHON_SRC)/python-mqtt_packet.c -o python-mqtt_packet.o

libemqtt-python.o: $(SRC)/libemqtt.c $(INC)/libemqtt.h
	$(CC) $(PYTHON_CCFLAGS) -c $(SRC)/libemqtt.c -o libemqtt-python.o


python3:
	@echo "Not yet available. Sorry!"





libemqtt.a: libemqtt.o
	$(AR) rcs libemqtt.a libemqtt.o

libemqtt.o: $(SRC)/libemqtt.c $(INC)/libemqtt.h
	$(CC) $(CCFLAGS) -c $(SRC)/libemqtt.c -o libemqtt.o





all: c python





clean:
	rm -f *.o libemqtt.a

dist-clean: clean
	rm -f $(CLIENT)/pub $(CLIENT)/sub $(CLIENT)/libemqtt.so
