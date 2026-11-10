mkdir build
cmake -DUSE_TLS=On -DCMAKE_BUILD_TYPE=Release -DPORTABLE_BUILD=On -DCOMPILER_RUNTIME_LIB_DIR="C:/mingw64/x86_64-w64-mingw32/lib" -G Ninja -B build
cmake --build build --clean-first
cpack --config ./build/CPackConfig.cmake -G ZIP