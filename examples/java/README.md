# libicsneo Java Example

This is an example console application which uses icsneojava to connect to an Intrepid Control Systems hardware device. It has both interactive and simple examples for sending and receiving CAN & CAN FD traffic.

## Cloning

This will create a copy of the repository on your local machine.

Run:

```shell
git clone https://github.com/intrepidcs/libicsneo-examples -b v0.2.0-dev --recursive
```

Alternatively, if you cloned without the `--recursive flag`, you must enter the `libicsneo-examples` folder and run the following:

```shell
git submodule update --recursive --init
```

If you haven't done this, `third-party/libicsneo` will be empty and you won't be able to build!

## Windows

### Building the DLL's

#### icsneoc

First, we are going to build the icsneoc library into a .dll file that is used by the Java wrapper to access the library functions.

1. Change directories to the `libicsneo-examples/third-party/libicsneo` folder.
2. Create a build directory by running `mkdir -p build`
3. Enter the build directory with `cd build`
4. Run `cmake ..` to generate your Makefile.
    * Hint! Running `cmake -DCMAKE_BUILD_TYPE=Debug ..` will generate the proper scripts to build debug, and `cmake -DCMAKE_BUILD_TYPE=Release ..` will generate the proper scripts to build with all optimizations on.
5. Run `cmake --build .` to build the library.
    * Hint! Speed up your build by using multiple processors! Use `make -j#` where `#` is the number of cores/threads your system has plus one. For instance, on a standard 8 thread Intel i7, you might use `-j9` for an ~8x speedup.
6. The `icsneoc.dll` file will be generated in `libicsneo-examples/third-party/libicsneo/build/Debug`. Move this file to the `/C/Windows/System32` folder.

#### icsneojava

Next, we are going to build the wrapper functions into a .dll file that is used to access the library functions in Java.

1. Change directories to the `libicsneo-examples/libicsneojava-example` folder.
2. Create a build directory by running `mkdir -p build`
3. Enter the build directory with `cd build`
4. Run `cmake ..` to generate your Makefile.
    * Hint! Running `cmake -DCMAKE_BUILD_TYPE=Debug ..` will generate the proper scripts to build debug, and `cmake -DCMAKE_BUILD_TYPE=Release ..` will generate the proper scripts to build with all optimizations on.
5. Run `cmake --build .` to build the library.
    * Hint! Speed up your build by using multiple processors! Use `make -j#` where `#` is the number of cores/threads your system has plus one. For instance, on a standard 8 thread Intel i7, you might use `-j9` for an ~8x speedup.
6. The `icsneojava.dll` file will be generated in `libicsneo-examples/libicsneojava-example/build/Debug`. Move this file to the `/C/Windows/System32` folder.

### Building and running the example program

1. Change directories to the `libicsneo-examples/libicsneojava-example/src` folder.
2. Run `javac Run.java`
3. Run `java Run`

## Ubuntu 18.04 LTS

### Building the .so's

#### icsneoc

1. Install dependencies with `sudo apt update` then `sudo apt install build-essential cmake libusb-1.0-0-dev libpcap0.8-dev`
2. Change directories to `libicsneo-examples/third-party/libicsneo` and create a build directory by running `mkdir -p build`
3. Enter the build directory with `cd build`
4. Run `cmake ..` to generate your Makefile.
    * Hint! Running `cmake -DCMAKE_BUILD_TYPE=Debug ..` will generate the proper scripts to build debug, and `cmake -DCMAKE_BUILD_TYPE=Release ..` will generate the proper scripts to build with all optimizations on.
5. Run `make` to build the library.
    * Hint! Speed up your build by using multiple processors! Use `make -j#` where `#` is the number of cores/threads your system has plus one. For instance, on a standard 8 thread Intel i7, you might use `-j9` for an ~8x speedup.
6. Run `sudo cp libicsneoc.so /usr/lib` so that it can be found via the default ubuntu .so search path. For more information, see the [ld.so.8 man page](http://man7.org/linux/man-pages/man8/ld.so.8.html).

#### icsneojava

Next, we are going to build the wrapper functions into a .so file that is used to access the library functions in Java.

1. Change directories to the `libicsneo-examples/libicsneojava-example` folder.
2. Create a build directory by running `mkdir -p build`
3. Enter the build directory with `cd build`
4. Run `cmake ..` to generate your Makefile.
    * Hint! Running `cmake -DCMAKE_BUILD_TYPE=Debug ..` will generate the proper scripts to build debug, and `cmake -DCMAKE_BUILD_TYPE=Release ..` will generate the proper scripts to build with all optimizations on.
5. Run `cmake --build .` to build the library.
    * Hint! Speed up your build by using multiple processors! Use `make -j#` where `#` is the number of cores/threads your system has plus one. For instance, on a standard 8 thread Intel i7, you might use `-j9` for an ~8x speedup.
6. The `icsneojava.so` file will be generated in `libicsneo-examples/libicsneojava-example/build/Debug`. Run `sudo cp libicsneojava.so /usr/lib` so that it can be found via the default ubuntu .so search path. For more information, see the [ld.so.8 man page](http://man7.org/linux/man-pages/man8/ld.so.8.html).

### Building and running the example program

1. Change directories to the `libicsneo-examples/libicsneojava-example/src` folder.
2. Run `javac Run.java`
3. Run `java Run`
