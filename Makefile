CC = g++
CFLAGS = -o bin/main -g
LIBS = -lpthread -lncurses
SRC_DIR = src
BIN_DIR = bin
INC_DIR = include

$(BIN_DIR)/%: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

all: $(BIN_DIR)/main

run: clean $(BIN_DIR)/main
	./$(BIN_DIR)/main 29.5

test: clean $(BIN_DIR)/unit
	./$(BIN_DIR)/unit

setup_can:
	ip link set can0 txqueuelen 1000
	ip link set can0 type can bitrate 1000000
	ip link set up can0

setup_vcan:
	modprobe vcan
	ip link add dev vcan0 type vcan
	ip link set up vcan0

clean:
	rm -f bin/*

.PHONY: all run test setup_can setup_vcan clean
