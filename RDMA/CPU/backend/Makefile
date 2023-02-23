target = backend ../driver/driver.o ../driver/flow_blocks.o ../driver/rdma_driver.o ../driver/rdma_common.o

CC = g++ 
LD_LIBRARY = $(shell pkg-config --libs libdpdk librdmacm libibverbs)
LD_INCLUDE = -I../
CFLAGS = $(shell pkg-config --cflags libdpdk librdmacm libibverbs) -Og -g -fpermissive -mcmodel=large #-DDEBUG

all: backend

backend: backend.cc ../driver/driver.o ../driver/flow_blocks.o ../driver/rdma_driver.o ../driver/rdma_common.o
	$(CC) $(CFLAGS) $(LD_INCLUDE) $^ -o $@ $(LD_LIBRARY)

../driver/driver.o: ../driver/driver.cc
	$(CC) $(CFLAGS) $(LD_INCLUDE) $^ -c -o $@ $(LD_LIBRARY)

../driver/flow_blocks.o: ../driver/flow_blocks.cc
	$(CC) $(CFLAGS) $(LD_INCLUDE) $^ -c -o $@ $(LD_LIBRARY)

../driver/rdma_driver.o: ../driver/rdma_driver.cc
	$(CC) $(CFLAGS) $(LD_INCLUDE) $^ -c -o $@ $(LD_LIBRARY)

../driver/rdma_common.o: ../driver/rdma_common.cc
	$(CC) $(CFLAGS) $(LD_INCLUDE) $^ -c -o $@ $(LD_LIBRARY)

.PHONY: clean
clean:
	-rm $(target)
