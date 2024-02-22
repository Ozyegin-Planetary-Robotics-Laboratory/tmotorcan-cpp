# TMotor C++ SocketCAN Library

The purpose of this header-only motors library is to accommodate a version of the Python TMotorCAN in C++ such that research & development can be sped up by allowing for finer integration in existing control.
Feel free to make pull requests to the repository.

## How to Run

The project uses Makefile for compiling a demonstrative example. First off, connect an USB-to-CAN adapter to your computer or otherwise connect to the CAN controller of your development platform, then assuming you are using 1MBPS bitrate, do as follows,

`make setup`

This is going to configure and setup the 'can0' CAN Bus network in your operating system. Afterwards, you can compile and run the code you modified for your own experimental setup by,

`make test`

To simply compile and not run the project, you may do,

`make all`

## Development

Feel free to add more to this library and make contributions, we welcome any help in making the tedious lower-level interface crafting work easier, especially for new comers into the robotics community.