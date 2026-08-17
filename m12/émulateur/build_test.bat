@echo off
cls
cmake -DUSE_TLS=On -DUSE_MBED_TLS=On -DCMAKE_BUILD_TYPE=RelWithDebInfo -DFETCHCONTENT_FULLY_DISCONNECTED=On -DCMAKE_INSTALL_PREFIX="./install" -DUSE_DECOMP_TOOLS=Off -DUSE_SSE2=On -DUSE_AVX2=On -DPORTABLE_BUILD=On -G Ninja -B build
cmake --build build --clean-first
cmake --install build
"%~dp0/install/M12.exe"