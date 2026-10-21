@setlocal
@echo off

call "%VCVARS64_2022%"

set "SKBUILD_CMAKE_DEFINE=LIBICSNEO_ENABLE_BOOTLOADER=ON;ICSPB_BOOTSTRAP_DIR=%CD%\icspb;FETCHCONTENT_BASE_DIR=%CD%\fc"
set "PATH=%CD%\fc\libicsbl-src\bin;%PATH%"

python.exe -m venv env || exit /b 1
call env\Scripts\Activate.bat || exit /b 1
python.exe -m pip install cibuildwheel || exit /b 1
python.exe -m cibuildwheel --output-dir wheelhouse --platform windows || exit /b 1
