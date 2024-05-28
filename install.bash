#!/bin/bash

rm -rf bin/
rm -rf lib/
mkdir bin
mkdir lib

g++ -c src/tmotor.cpp -o src/tmotor.o 
ar -rcs lib/libtmotor.a src/tmotor.o 
g++ -o bin/main src/main.cpp -L./lib/ -ltmotor -lpthread -lncurses

rm src/tmotor.o

cp bin/main /usr/local/bin/tmotorui
cp lib/libtmotor.a /usr/local/lib/
cp include/tmotor.hpp /usr/local/include/