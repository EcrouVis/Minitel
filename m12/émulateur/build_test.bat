@echo off
cls
cmake -DUSE_TLS=On -DUSE_MBED_TLS=On -DCMAKE_BUILD_TYPE=RelWithDebInfo -DFETCHCONTENT_FULLY_DISCONNECTED=On -DCMAKE_INSTALL_PREFIX="./install" -DCOMPILER_RUNTIME_LIB_DIR="C:/mingw64/x86_64-w64-mingw32/lib" -DUSE_DECOMP_TOOLS=Off -DPORTABLE_BUILD=On -G Ninja -B build
cmake --build build --clean-first
cmake --install build
"%~dp0/install/M12.exe"