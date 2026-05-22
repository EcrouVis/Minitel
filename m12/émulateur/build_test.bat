@echo off
cls
cd /D "%~dp0"
::g++ -c -I./include -I./include/imgui ./test/test.cpp ./src/glad.c ./src/imgui/backends/*.cpp ./src/imgui/*.cpp ./src/*.cpp 
::g++ -c -I./include -I./include/imgui test.cpp
::g++ *.o -L./lib -lglfw3 -lgdi32 -o ./test/test.exe
::gcc -I./include -I./include/imgui ./test/test.cpp ./src/glad.c ./src/circuit/SRAM_64k/*.cpp ./src/imgui/backends/*.cpp ./src/imgui/*.cpp ./src/*.cpp -L./lib -lglfw3 -lgdi32 -lstdc++ -o ./test/test.exe -D_GLIBCXX_DEBUG -D_GLIBXX_DEBUG_PEDANTIC -g -Wall -Wextra -O2 -Wno-unused-parameter
gcc -I./include -I./include/imgui -I./include/miniaudio -I./include/cJSON -I"C:/Program Files (x86)/LibreSSL/include" -I"C:/Program Files (x86)/ixwebsocket/include" ./test/test.cpp ./obj/*.o ./src/circuit/CPLD/*.cpp ./src/circuit/SRAM_64k/*.cpp ./src/circuit/ROM_256k/*.cpp ./src/circuit/TS9347/*.cpp ./src/circuit/80C32/*.cpp ./src/*.cpp ./src/miniaudio/miniaudio.c ./src/cJSON/cJSON.c -L./lib -L"C:/Program Files (x86)/ixwebsocket/lib" -L"C:/Program Files (x86)/LibreSSL/lib" -lixwebsocket -ltls -lssl -lcrypto -lcrypt32 -lbcrypt -lglfw3 -lgdi32 -lstdc++ -lWs2_32 -static -o ./test/test.exe -D_GLIBCXX_DEBUG -D_GLIBXX_DEBUG_PEDANTIC -g -Wall -Wextra -O2 -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers -gdwarf-2 -DMA_NO_DECODING -DMA_NO_ENCODING -msse2 -mavx2
:: -mwindows -O3 -flto

::del test.o
::del *.o
cd test
echo run test
test.exe
cd /D "%~dp0"