call "%VCVARS64%"

REM clean intermediate directories
rmdir /s /q build
mkdir build

REM build
cd build
set CXXFLAGS=/WX
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLIBICSNEO_BUILD_TESTS=ON ..
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
