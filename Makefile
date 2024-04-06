CC = g++
CFLAGS = -o bin/main
LIBS = -lpthread -lncurses
SRC_DIR = src
BIN_DIR = bin


$(BIN_DIR)/main: $(SRC_DIR)/client.cpp $(SRC_DIR)/ak60.hpp
	$(CC) $(CFLAGS) $(SRC_DIR)/client.cpp $(SRC_DIR)/ak60.hpp $(LIBS)

all: $(BIN_DIR)/main

test: $(BIN_DIR)/main
	./$(BIN_DIR)/main

setup_can:
	ip link set can0 txqueuelen 1000
	ip link set can0 type can bitrate 1000000
	ip link set up can0

setup_vcan:
	modprobe vcan
	ip link add dev vcan0 type vcan
	ip link set up vcan0

clean:
	rm bin/*

.PHONY: all test setup_can setup_vcan clean
