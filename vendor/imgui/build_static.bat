@echo off

set BasePath=%~dp0
set VendorPath=%BasePath%\..

set CFlags=/c /FeStella.exe /Zi /DEBUG:FULL /std:c++17 -D_CRT_SECURE_NO_WARNINGS -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DGLEW_NO_GLU
set LDFlags=/lib /OUT:imgui.lib /nologo /SUBSYSTEM:WINDOWS
set Includes=/I %BasePath%imgui /I ../../GLFW/include /I ../../GLEW/include
set ImGuiCPPs=%BasePath%imgui\imgui.cpp %BasePath%imgui\imgui_draw.cpp %BasePath%imgui\imgui_impl_glfw.cpp %BasePath%imgui\imgui_impl_opengl3.cpp %BasePath%imgui\imgui_widgets.cpp
set ImGuiObjs=imgui.obj imgui_draw.obj imgui_impl_glfw.obj imgui_impl_opengl3.obj imgui_widgets.obj

if not exist lib mkdir lib

REM remove the pushd/popd! it's more annoyance than it's worth
pushd lib
cl %CFlags% %Includes% %ImGuiCPPs%
link %LDFlags% %ImGuiObjs%
popd