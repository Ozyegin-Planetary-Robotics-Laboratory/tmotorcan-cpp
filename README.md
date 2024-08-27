# TMotor C++ SocketCAN Library

The purpose of this library is to create a high-level interface for communicating with TMotor AK series actuators using SocketCAN. It should work on any system that implements SocketCAN interface as the object relies on a CAN socket to communicate with it's designated motor.

The project consists of two programs and a testing routine, if you're interesting in writing a control application for operating AK series servo motors and conducting manual control for experiments, then you are highly suggested to follow the below build & install instructions while ignoring the unit testing.

## Build & Installation

The project uses CMake for building and installation, first, clone the repository into a local folder using `git clone`. Then run the below commands inside the repository.

```bash
mkdir build && cd build
cmake ..
sudo make install
```

Once installation is complete, you may test-control any AK motor by using the TUI that comes installed alongside it.

```bash
tmotorui <reduction> <can_interface>
```

You may also access the motor manager class by including the "tmotor.hpp" header in your project, and using the appropriate compiler flags or directives to link your library to this project.

```cpp
#include <tmotor.hpp>
```

If you are using g++ or a Makefile, the correct flag is -ltmotor

```bash
g++ -o main main.cpp -ltmotor
```

or if you are using CMake simply add the below directive after the `add_executable()` directive,

```cmake
target_link_libraries(<target> PRIVATE tmotor)
```

## Development

Feel free to add issues and make more contributions to this project, we welcome any help. Though we might have CI pipeline, while working with the project, you might want to manually unit test the code. In order to run the unit tests, you need to build the project with the `BUILD_TEST` argument set. You may follow the below instructions.

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTS=on
make all
cd tests
ctest -V # verbose
```

Copy paste the above script line by line, and you will have compiled the tests cases and ran them.