# TMotor C++ SocketCAN Library

The purpose of this library is to create a high-level interface for communicating with TMotor AK series actuators using SocketCAN. 

## Example UI

The project uses a Bash script for compiling a demonstrative application as well as the library then installing it on the local computer.

```bash
bash install.bash
```

Once installation is complete, you may test-control any AK motor by using the graphical user interface.

```bash
tmotorui <reduction>
```

You may also access the motor class by including the "tmotor.hpp" header in your project, and using the appropriate compiler flag.

```cpp
#include <tmotor.hpp>
```

```bash
g++ -o main main.cpp -ltmotor
```

## Development

Feel free to add more to this library and make contributions, we welcome any help in making the tedious lower-level interface crafting work easier, especially for new comers into the robotics community.
