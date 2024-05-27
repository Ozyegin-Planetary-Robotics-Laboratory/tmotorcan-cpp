# TMotor C++ SocketCAN Library

The purpose of this header-only motors library is to accommodate an analogy of the Python TMotorCAN in C++ such that research & development can be sped by giving more freedom to new-coming developers that want to get right into working with these motors in C++.

## Example UI

The project uses Makefile for compiling a demonstrative example. First off, connect an USB-to-CAN adapter to your computer that accommodates a SocketCAN network interface and setup the network interface.

`make all`

Once compilation is complete, you may test the performance of the library by using the graphical user interface.

`./bin/main <reduction>`

To install the application alongside the header only library, simply do,

`make install`

## Development

Feel free to add more to this library and make contributions, we welcome any help in making the tedious lower-level interface crafting work easier, especially for new comers into the robotics community.
