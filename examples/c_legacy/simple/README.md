# libicsneo C Example

This is an example console application that uses the icsneoc library to control an Intrepid Control Systems hardware device.

## Cloning

This will create a copy of the repository on your local machine.

Run:

```shell
git clone https://github.com/intrepidcs/libicsneo-examples -b v0.2.0-dev --recursive
```

Alternatively, if you cloned without the `--recursive` flag, you must enter the `libicsneo-examples` folder and run the following:

```shell
git submodule update --recursive --init
```

If you haven't done this, `third-party/libicsneo` will be empty and you won't be able to build!

## Windows using Visual Studio 2017+

### Building the DLL

First, we are going to build the icsneoc library into a .dll file that we can later use in order to access the library functions.

1. Launch Visual Studio and open the `libicsneo-examples` folder.
2. Choose `File->Open->Cmake...`
3. Navigate to `third-party/libicsneo` and select the `CMakeLists.txt` there.
4. Visual Studio will process the CMake project.
5. Select `Build->Rebuild All`
6. Visual Studio will generate the `icsneoc.dll` file, which can then be found by selecting `Project->Cmake Cache (x64-Debug Only)->Open in Explorer`. If the file cannot be found, search in `libicsneo-examples/third-party/libicsneo/out/build/x64-Debug` and double-check that the build succeeded in step 5.
7. Move the `icsneoc.dll` file to the `/C/Windows/System32` folder. This will allow it to be found by icsneo_init(), which loads all the library functions.
    * Alternatively, the `icsneoc.dll` file can be placed in the same directory as `libicsneoc-example.exe`, which is typically `libicsneo-examples/libicsneoc-example/out/build/x64-Debug`, although this is not recommended. For more information, refer to [the Microsoft documentation](https://docs.microsoft.com/en-us/windows/desktop/dlls/dynamic-link-library-search-order).

### Building the example program

Although the example program will build without successfully completing the steps above, it will exit immediately upon running due to a failure to load any library functions.

1. Choose `File->Open->Cmake...`
2. Navigate to `libicsneo-examples/libicsneoc-example` and select the `CMakeLists.txt` there.
3. Visual Studio will process the CMake project.
4. Select `Build->Rebuild All`
5. Click on the dropdown arrow attached to the green play button (labelled "Select Startup Item") and select `libicsneoc-example.exe`
6. Click on the green play button to run the example.

## Ubuntu 18.04 LTS

### Building the .so

First, we are going to build the icsneoc library into a .so file that we can later use in order to access the library functions.

1. Install dependencies with `sudo apt update` then `sudo apt install build-essential cmake libusb-1.0-0-dev libpcap0.8-dev`
2. Change directories to `libicsneo-examples/third-party/libicsneo` and create a build directory by running `mkdir -p build`
3. Enter the build directory with `cd build`
4. Run `cmake ..` to generate your Makefile.
    * Hint! Running `cmake -DCMAKE_BUILD_TYPE=Debug ..` will generate the proper scripts to build debug, and `cmake -DCMAKE_BUILD_TYPE=Release ..` will generate the proper scripts to build with all optimizations on.
5. Run `make` to build the library.
    * Hint! Speed up your build by using multiple processors! Use `make -j#` where `#` is the number of cores/threads your system has plus one. For instance, on a standard 8 thread Intel i7, you might use `-j9` for an ~8x speedup.
6. Run `sudo cp libicsneoc.so /usr/lib` so that it can be found via the default ubuntu .so search path. For more information, see the [ld.so.8 man page](http://man7.org/linux/man-pages/man8/ld.so.8.html).

### Building the example program

Although the example program will build without successfully completing the steps above, it will exit immediately upon running due to a failure to load any library functions.

1. Change directories to `libicsneo-examples/libicsneoc-example`
2. Create a build directory by running `mkdir -p build`
3. Enter the build directory with `cd build`
4. Run `cmake ..` to generate your Makefile.
    * Hint! Running `cmake -DCMAKE_BUILD_TYPE=Debug ..` will generate the proper scripts to build debug, and `cmake -DCMAKE_BUILD_TYPE=Release ..` will generate the proper scripts to build with all optimizations on.
5. Run `make` to build the library.
    * Hint! Speed up your build by using multiple processors! Use `make -j#` where `#` is the number of cores/threads your system has plus one. For instance, on a standard 8 thread Intel i7, you might use `-j9` for an ~8x speedup.
6. Run `sudo ./libicsneoc-example` to run the example.
    * Hint! In order to run without sudo, you will need to set up the udev rules. Copy `libicsneo-examples/third-party/libicsneo/99-intrepidcs.rules` to `/etc/udev/rules.d`, then run `udevadm control --reload-rules && udevadm trigger` afterwards. While the program will still run without setting up these rules, it will fail to open any devices.

## macOS

Instructions coming soon&trade;
