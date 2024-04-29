# comm-example

This is a simple C++ example on how to set-up a client/server communication
protocol.

Based on [this page](https://www.geeksforgeeks.org/socket-programming-cc/).

# building

mkdir build
cd build && cmake ../
make

# example usage

./server 8000 &
./client 502 8000 &
./client 2432 8000     // <- in another terminal, for instance

## requirements

  The library is in C++ and uses CMake (v 3.5+) as a build system.
