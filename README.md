# comm-example

This is a simple C++ example on how to set-up a client/server communication
protocol.

Based on [this page](https://www.geeksforgeeks.org/socket-programming-cc/).

# building

mkdir build
cd build && cmake ../
make
./server 8000
./client 8000

## requirements

  The library is in C++ and uses CMake (v 3.5+) as a build system.
