# libicsneo C++ Example

This is an example console application which uses libicsneo to connect to an Intrepid Control Systems hardware device. It has both interactive and simple examples for sending and receiving CAN & CAN FD traffic.

## Building

This example shows how to use the C++ version of libicsneo with CMake. It will build libicsneo along with your project.

First, you need to clone the repository onto your local machine. Run:

```shell
git clone https://github.com/intrepidcs/libicsneo-examples --recursive
```

Alternatively, if you cloned without the `--recursive flag`, you must enter the `libicsneo-examples` folder and run the following:

```shell
git submodule update --recursive --init
```

If you haven't done this, `third-party/libicsneo` will be empty and you won't be able to build!

### Windows using Visual Studio 2017+

1. Launch Visual Studio and open the `libicsneo-examples` folder.
2. Choose `File->Open->CMake...`
3. Navigate to the `libicsneocpp-example` folder and select the `CMakeLists.txt` there.
4. Visual Studio will process the CMake project.
5. Choose the dropdown attached to the green play button (labelled "select startup item...") in the toolbar.
6. Select `libicsneocpp-simple-example.exe`
7. Press the green play button to compile and run the example.

### Ubuntu 18.04 LTS

1. Install dependencies with `sudo apt update` then `sudo apt install build-essential cmake libusb-1.0-0-dev libpcap0.8-dev`
2. Change directories to your `libicsneo-examples/libicsneocpp-example` folder and create a build directory by running `mkdir -p build`
3. Enter the build directory with `cd build`
4. Run `cmake ..` to generate your Makefile.
    * Hint! Running `cmake -DCMAKE_BUILD_TYPE=Debug ..` will generate the proper scripts to build debug, and `cmake -DCMAKE_BUILD_TYPE=Release ..` will generate the proper scripts to build with all optimizations on.
5. Run `make libicsneocpp-interactive-example` to build.
    * Hint! Speed up your build by using multiple processors! Use `make libicsneocpp-interactive-example -j#` where `#` is the number of cores/threads your system has plus one. For instance, on a standard 8 thread Intel i7, you might use `-j9` for an ~8x speedup.
6. Now run `sudo ./libicsneocpp-interactive-example` to run the example.
    * Hint! In order to run without sudo, you will need to set up the udev rules. Copy `libicsneo-examples/third-party/libicsneo/99-intrepidcs.rules` to `/etc/udev/rules.d`, then run `udevadm control --reload-rules && udevadm trigger` afterwards. While the program will still run without setting up these rules, it will fail to open any devices.
7. If you wish to run the simple example instead, replace any instances of "interactive" with "simple" in steps 5 and 6.

### macOS

Instructions coming soon&trade;
