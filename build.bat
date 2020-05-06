@echo off

set BasePath=%~dp0
set VendorPath=%BasePath%vendor

set CFlags=/FeStella.exe /Zi /DEBUG:FULL /std:c++17 -D_CRT_SECURE_NO_WARNINGS
set LDFlags=/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib /ignore:4099 /nologo
set Includes=/I %VendorPath%\GLEW\include /I %VendorPath%\GLFW\include /I %VendorPath%\glm /I %VendorPath%\imgui /I %VendorPath%\sci.h /I %VendorPath%\stb /I %VendorPath%\siv
set Libs=%VendorPath%\GLEW\lib\glew32s.lib %VendorPath%\GLFW\lib\glfw3.lib %VendorPath%\imgui\lib\imgui.lib opengl32.lib gdi32.lib user32.lib shell32.lib

REM we use goto because blocks don't work; thx microsoft
if exist %BasePath%\vendor\imgui\lib\imgui.lib goto :imgui_exists
pushd %BasePath%\vendor\imgui
call build_static.bat
popd
:imgui_exists

if not exist build mkdir build

REM remove the pushd/popd! it's more annoyance than it's worth
pushd build
cl %CFlags% %Includes% %BasePath%src\main.cpp /link %LDFlags% %Libs%
popd