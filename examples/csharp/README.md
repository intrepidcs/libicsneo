# libicsneo C# Example

This is an example console application that uses the icsneocsharp library to control an Intrepid Control Systems hardware device.

## Cloning

This will create a copy of the repository on your local machine.

Run:

```shell
git clone https://github.com/intrepidcs/libicsneo-examples --recursive
```

Alternatively, if you cloned without the `--recursive` flag, you must enter the `libicsneo-examples` folder and run the following:

```shell
git submodule update --recursive --init
```

If you haven't done this, `third-party/libicsneo` will be empty and you won't be able to build!

## Windows using Visual Studio 2017+

### Building the DLLs

#### icsneoc

First, we are going to build the icsneoc library into a .dll file that is used by the C# wrapper to access the library functions.

1. Launch Visual Studio and open the `libicsneo-examples` folder.
2. Choose `File->Open->Cmake...`
3. Navigate to `third-party/libicsneo` and select the `CMakeLists.txt` there.
4. Visual Studio will process the CMake project.
5. Select `Build->Rebuild All`
6. Visual Studio will generate the `icsneoc.dll` file, which can then be found by selecting `Project->Cmake Cache (x64-Debug Only)->Open in Explorer`. If the file cannot be found, search in `libicsneo-examples/third-party/libicsneo/out/build/x64-Debug` and double-check that the build succeeded in step 5.
7. Move the `icsneoc.dll` file to the `/C/Windows/System32` folder.

#### icsneocsharp

Next, we are going to build the wrapper functions into a .dll file that is used to access the library functions in C#.

1. Launch a terminal window and change directories into `libicsneo-examples/libicsneocsharp-example`, then create a build directory by running `mkdir -p build`
2. Enter the build directory with `cd build`
3. Run `cmake ..`
4. Run `cmake --build .`
5. The `icsneocsharp.dll` file will be generated in `libicsneo-examples/libicsneocsharp-example/build/Debug`
6. Move the `icsneocsharp.dll` file to the `/C/Windows/System32` folder.

### Building the example program

1. Choose `File->Open->Project/Solution...`
2. Navigate to `libicsneo-examples/libicsneocsharp-example` and select the `libicsneocsharp-example.sln` there.
3. Visual Studio will process the project.
4. Select `Build->Rebuild Solution`
5. Click on the dropdown arrow attached to the green play button (labelled "Select Startup Item") and select `libicsneocsharp-example`
6. Click on the green play button to run the example.
