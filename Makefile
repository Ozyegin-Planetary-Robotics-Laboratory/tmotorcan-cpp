CC = g++
CFLAGS = -o bin/main -g
LIBS = -lpthread -lncurses
SRC_DIR = src
BIN_DIR = bin
INC_DIR = include

$(BIN_DIR)/%: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

all: $(BIN_DIR)/main

clean:
	rm -f bin/*

install: all
	cp bin/main /usr/local/bin/tmotorcan-cpp

.PHONY: all clean install
