@echo off
 
set CFlags=/FeStella.exe /Zi /std:c++17 -D_CRT_SECURE_NO_WARNINGS
set LDFlags=/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib /ignore:4099 
set Includes=/I ../vendor/GLEW/include /I ../vendor/GLFW/include /I ../vendor/glm /I ../vendor/imgui /I ../vendor/sci.h
set Libs=../vendor/GLEW/lib/glew32s.lib ../vendor/GLFW/lib/glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib
set ImGuiCPPs=../vendor/imgui/imgui/imgui.cpp ../vendor/imgui/imgui/imgui_draw.cpp ../vendor/imgui/imgui/imgui_impl_glfw.cpp ../vendor/imgui/imgui/imgui_impl_opengl3.cpp ../vendor/imgui/imgui/imgui_widgets.cpp

mkdir build
pushd build
cl %CFlags% %Includes% ../src/main.cpp %ImGuiCPPs% %Libs% /link %LDFlags%
popd