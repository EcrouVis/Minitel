@echo off
cd /D "%~dp0"
g++ -c -I./include -I./include/imgui ./test/test.cpp ./src/glad.c ./src/imgui/backends/*.cpp ./src/imgui/*.cpp ./src/thread_messaging.cpp
::g++ -c -I./include -I./include/imgui test.cpp
g++ -L./lib test.o glad.o -lglfw3 -lgdi32 imgui_demo.o imgui.o imgui_impl_glfw.o imgui_impl_opengl3.o imgui_draw.o imgui_tables.o imgui_widgets.o thread_messaging.o -o ./test/test.exe
::del test.o
del *.o
cd test
test.exe
cd /D "%~dp0"