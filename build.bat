@echo off

ctime -begin stella.ctm

set BasePath=%~dp0
set VendorPath=%BasePath%vendor

set CFlags=/FeStella.exe /std:c++17 /Zi /DEBUG:FULL -DSTELLA_STATIC
set LDFlags=-nodefaultlib -subsystem:windows
set Includes=/I %
set Libs=opengl32.lib gdi32.lib user32.lib shell32.lib kernel32.lib
REM set CFlags=/FeStella.exe /std:c++17 -D_CRT_SECURE_NO_WARNINGS /Zi /DEBUG:FULL
REM set LDFlags=/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib /ignore:4099 /NOLOGO /INCREMENTAL:NO /OPT:REF
REM set Includes=/I %VendorPath%\GLEW\include /I %VendorPath%\GLFW\include /I %VendorPath%\glm /I %VendorPath%\imgui /I %VendorPath%\sci.h /I %VendorPath%\stb /I %VendorPath%\rnd
REM set Libs=%VendorPath%\GLEW\lib\glew32s.lib %VendorPath%\GLFW\lib\glfw3.lib %VendorPath%\imgui\lib\imgui.lib opengl32.lib gdi32.lib user32.lib shell32.lib

REM we use goto because blocks don't work; thx microsoft
REM if exist %BasePath%\vendor\imgui\lib\imgui.lib goto :imgui_exists
REM pushd %BasePath%\vendor\imgui
REM call build_static.bat
REM popd
REM :imgui_exists

if not exist build mkdir build

REM remove the pushd/popd! it's more annoyance than it's worth
pushd build
cl %CFlags% %BasePath%src\win32_platform.cpp /link %LDFlags%
set LastError=%ERRORLEVEL%
REM cl %CFlags% %Includes% %BasePath%src\main.cpp /link %LDFlags% %Libs%
popd

ctime -end stella.ctm %LastError%