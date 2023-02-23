CC=g++
CFLAGS=$(shell pkg-config --cflags libdpdk librdmacm libibverbs) -O3 -g -fpermissive
LIBRARYS=$(shell pkg-config --libs libdpdk librdmacm libibverbs)
INCLUDES=-I../
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -c -o $@ $(LIBRARYS)

clean:
	$(RM) $(OBJECTS)

