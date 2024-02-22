CC = g++
CFLAGS = -o bin/main
LIBS = -lpthread
SRC_DIR = src

all: bin/main

bin/main: $(SRC_DIR)/ak60test.cpp $(SRC_DIR)/ak60.hpp
	$(CC) $(CFLAGS) $(SRC_DIR)/ak60test.cpp $(SRC_DIR)/ak60.hpp $(LIBS)

test: bin/main
	./bin/main

setup:
	ip link set can0 type can bitrate 1000000
	ip link set up can0

clean:
	rm -rf bin/*

.PHONY: all test setup clean
