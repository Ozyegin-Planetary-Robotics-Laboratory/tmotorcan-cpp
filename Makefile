CC = g++
CFLAGS = -o bin/main -g
LIBS = -lncurses
SRC_DIR = src
BIN_DIR = bin
INC_DIR = include

$(BIN_DIR)/%: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

all: $(BIN_DIR)/main

clean:
	rm -f bin/*

install: all
	sudo cp bin/main /usr/local/bin/tmotorui
	sudo cp include/TMotor.hpp /usr/local/include/TMotor.hpp

.PHONY: all clean install
