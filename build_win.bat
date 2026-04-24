@echo off
setlocal

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"

set "BUILD_DIR=%~dp0build\cmake-x64"
set "SFML_CMAKE_DIR=%SFML_DIR%\lib\cmake\SFML"
if "%SFML_DIR%"=="" set "SFML_CMAKE_DIR=C:\dev\libraries\SFML-3.0.1\lib\cmake\SFML"

cmake -S "%~dp0." -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 -DSFML_DIR="%SFML_CMAKE_DIR%"
if errorlevel 1 exit /b %errorlevel%

cmake --build "%BUILD_DIR%" --config "%CONFIG%" --parallel 8
exit /b %errorlevel%
