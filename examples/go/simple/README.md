libicsneoc2 simple Go example
====

This is a mirror of the icsneoc2 C simple example, written in Go.

Windows
====

- Install [msys64](https://www.msys2.org/) with gcc (`pacman -S mingw-w64-ucrt-x86_64-gcc`)
- Setup environment variables:
    - add `C:\msys64\ucrt64\bin` to `PATH`
        - Powershell: `$env:PATH += ";C:\msys64\ucrt64\bin"`
        - `gcc --version` should return a version now
    - enable cgo: `CGO_ENABLED = 1`
        - Powershell: `$env:CGO_ENABLED=1`
    - Set compiler to gcc
        - Powershell: `$env:CC="gcc"`
    - `icsneoc2.dll` should be in path (or inside this directory)
- `go run simple`
