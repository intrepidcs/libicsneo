REM clean intermediate directories
rmdir /s /q build
mkdir build

REM build
cd build
set CFLAGS=/WX
set CXXFLAGS=/WX
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLIBICSNEO_BUILD_UNIT_TESTS=ON -DLIBICSNEO_BUILD_SYSTEM_TESTS=ON -DLIBICSNEO_ENABLE_TCP=ON ..
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
