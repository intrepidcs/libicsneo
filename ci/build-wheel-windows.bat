@setlocal
@echo off

call "%VCVARS64_2022%"

python.exe -m venv env || exit /b 1
call env\Scripts\Activate.bat || exit /b 1
python.exe -m pip install cibuildwheel || exit /b 1
python.exe -m cibuildwheel --output-dir wheelhouse --platform windows || exit /b 1
