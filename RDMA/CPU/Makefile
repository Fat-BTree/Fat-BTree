CC=g++
CFLAGS=-std=c++11 -Og -Wall -pthread -I./ $(shell pkg-config --cflags libdpdk librdmacm libibverbs) -fpermissive
LIBRARYS=$(shell pkg-config --libs libdpdk librdmacm libibverbs)
LDFLAGS= -lpthread -ltbb -lhiredis
SUBDIRS=core db redis driver
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc) $(wildcard driver/*.cc)
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBRARYS)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)

.PHONY: $(SUBDIRS) $(EXEC)
