cmake_minimum_required(VERSION 3.23)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER gcc)
set( CMAKE_C_COMPILER gcc )
set(CMAKE_RC_COMPILER windres)

project(M12 VERSION 0.0.1 LANGUAGES CXX C RC)

set(CMAKE_BUILD_TYPE Debug)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ressources")
file(COPY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/TS9347/TS9347_Texture_Character_Set_Datasheet.bmp" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ressources/TS9347_Texture_Character_Set_Datasheet.bmp")
file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rom")
file(COPY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/ROM_256k/Bz6.rom" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rom/Bz6.rom")

add_executable(test)
SET_TARGET_PROPERTIES(test PROPERTIES OUTPUT_NAME "M12")

add_library(wicon OBJECT)
set_target_properties(wicon PROPERTIES LINKER_LANGUAGE CXX)

find_path(IXWEBSOCKET_INCLUDE_DIR ixwebsocket/IXWebSocket.h "C:/Program Files (x86)/ixwebsocket/include")
find_library(IXWEBSOCKET_LIBRARY ixwebsocket "C:/Program Files (x86)/ixwebsocket/lib")
add_library(ixwebsocket UNKNOWN IMPORTED)
set_target_properties(
	ixwebsocket
	PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${IXWEBSOCKET_INCLUDE_DIR}"
		IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
		IMPORTED_LOCATION "${IXWEBSOCKET_LIBRARY}"
)

set(LIBRESSL_ROOT_DIR "C:/Program Files (x86)/LibreSSL")
find_package(LibreSSL REQUIRED)

target_sources(wicon
	PRIVATE
		include/icon/app.rc
)

target_compile_options(test PRIVATE -static -Wall -Wextra -O2 -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers -msse2 -mavx2 -flto -g -gdwarf-2)

target_link_directories(test
	PUBLIC
		lib
)

target_link_libraries(test
	PRIVATE
		ixwebsocket
		LibreSSL::TLS
		LibreSSL::SSL
		LibreSSL::Crypto
		crypt32
		bcrypt
		
		glfw3
		gdi32
		stdc++
		Ws2_32
		
		wicon
)

target_compile_definitions(test
	PRIVATE
		_GLIBCXX_DEBUG
		_GLIBXX_DEBUG_PEDANTIC
		
		MA_NO_DECODING
		MA_NO_ENCODING
		
		CJSON_HIDE_SYMBOLS
)

target_sources(test
	PRIVATE
		src/main.cpp
		
		src/encoding.cpp
		src/FileAccess.cpp
		src/main_circuit.cpp
		src/main_video.cpp
		src/thread_messaging.cpp
		
		src/circuit/80C32/80C32.cpp
		src/circuit/CPLD/MBSL_4000FH5_5.cpp
		src/circuit/ROM_256k/ROM_256k.cpp
		src/circuit/SRAM_64k/SRAM_64k.cpp
		src/circuit/TS9347/TS9347.cpp
		
		src/imgui/imgui.cpp
		src/imgui/imgui_demo.cpp#not needed
		src/imgui/imgui_draw.cpp
		src/imgui/imgui_tables.cpp
		src/imgui/imgui_widgets.cpp
		src/imgui/backends/imgui_impl_glfw.cpp
		src/imgui/backends/imgui_impl_opengl3.cpp
		
		src/cJSON/cJSON.c
		
		src/glad/glad.c
		
		src/miniaudio/miniaudio.c
	
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS
		include
	FILES
		include/encoding.h
		include/FileAccess.h
		include/FileSelector.h
		include/FontAwesomeIcon.h
		include/GlobalState.h
		include/license.h
		include/m12_threads.h
		include/MemoryWindow.h
		include/Parameters.h
		include/thread_messaging.h
		
		include/circuit/80C32.h
		include/circuit/AudioBuffer.h
		include/circuit/BuzzerFilter.h
		include/circuit/clocks.h
		include/circuit/CPLD.h
		include/circuit/CRTBuffer.h
		include/circuit/Keyboard.h
		include/circuit/Latch.h
		include/circuit/PhoneLine.h
		include/circuit/PhoneLineWirring.h
		include/circuit/ROM_256k.h
		include/circuit/SpeakerFilter.h
		include/circuit/SRAM_64k.h
		include/circuit/TS7514.h
		include/circuit/TS9347.h
		include/circuit/WatchdogTimer.h
		include/circuit/debug/dbg_80C32.h
		include/circuit/debug/decomp_m12_rom.h
		include/circuit/debug/IOLogger.h
		include/circuit/DIN5/MinitelNetwork.h
		include/circuit/DIN5/vdt.h
		include/circuit/RTC/RTCNetwork.h
		
		include/io/CRTRenderer.h
		include/io/KeyboardIndicator.h
		include/io/KeyboardInput.h
		include/io/NotificationServer.h
		include/io/PrinterOutput.h
		
		include/icon/M12_icon_square.png.h
		
		include/stb_image.h
		include/stb_image_write.h
		
		include/cJSON/cJSON.h
		
		include/glad/glad.h
		
		include/GLFW/glfw3.h
		include/GLFW/glfw3native.h
		
		include/imgui/imconfig.h
		include/imgui/imgui.h
		include/imgui/imgui_impl_glfw.h
		include/imgui/imgui_impl_opengl3.h
		include/imgui/imgui_impl_opengl3_loader.h
		include/imgui/imgui_internal.h
		include/imgui/imstb_rectpack.h
		include/imgui/imstb_textedit.h
		include/imgui/imstb_truetype.h
		
		include/KHR/khrplatform.h
		
		include/miniaudio/miniaudio.h
		
	PUBLIC
	FILE_SET imguiOnlyHeaders
	TYPE HEADERS
	BASE_DIRS
		include/imgui
	FILES
		include/imgui/imconfig.h
		include/imgui/imgui.h
		include/imgui/imgui_impl_glfw.h
		include/imgui/imgui_impl_opengl3.h
		include/imgui/imgui_impl_opengl3_loader.h
		include/imgui/imgui_internal.h
		include/imgui/imstb_rectpack.h
		include/imgui/imstb_textedit.h
		include/imgui/imstb_truetype.h
		
	PUBLIC
	FILE_SET cJSONOnlyHeader
	TYPE HEADERS
	BASE_DIRS
		include/cJSON
	FILES
		include/cJSON/cJSON.h
		
	PUBLIC
	FILE_SET miniaudioOnlyHeader
	TYPE HEADERS
	BASE_DIRS
		include/miniaudio
	FILES
		include/miniaudio/miniaudio.h
)