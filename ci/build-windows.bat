@setlocal
@echo off

mkdir build >nul 2>&1

cmake -GNinja -Bbuild -DCMAKE_BUILD_TYPE=Release -DLIBICSNEO_BUILD_UNIT_TESTS=ON ^
    -DLIBICSNEO_ENABLE_TCP=ON || exit /b 1

cmake --build build || exit /b 1
