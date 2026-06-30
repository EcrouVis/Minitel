cmake_minimum_required(VERSION 3.23)

include(FetchContent)

set(CMAKE_COLOR_DIAGNOSTICS On)
set(CMAKE_COLOR_MAKEFILE On)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_C_STANDARD 17)
set( CMAKE_C_COMPILER gcc )

project(M12 VERSION 0.0.1 LANGUAGES CXX C)



set(CMAKE_BUILD_TYPE RelWithDebInfo)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}" CACHE PATH "Force path to set CMAKE_INSTALL_PREFIX" FORCE)
endif()



file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")



add_executable(m12)
set_target_properties(m12 PROPERTIES OUTPUT_NAME "M12")
add_library(miniaudio STATIC)
add_library(cjson STATIC)
add_library(glloader STATIC)
add_library(imgui STATIC)





set(OPT_LIB )
set(OPT_DEF )





option(USE_EXISTING_IXWEBSOCKET_LIB "Use existing IXWebSocket library" Off)
option(USE_TLS "Use TLS for secure websockets" Off)
option(USE_OPEN_SSL "Use OpenSSL" Off)
option(USE_LIBRE_SSL "Use LibreSSL" Off)
option(USE_MBED_TLS "Use MbedTLS" Off)
option(USE_DECOMP_TOOLS "Whether to compile with tools to help decompile the ROM" Off)





if (USE_DECOMP_TOOLS)
	list(APPEND 
		OPT_DEF
			M12_USE_DECOMP_TOOLS
	)
endif()

#get libs

#copied from IXWebsocket
if (USE_TLS)
    if (WIN32)
      if (NOT USE_OPEN_SSL AND NOT USE_LIBRE_SSL) # unless we want something else
        set(USE_MBED_TLS ON)
      endif()
    else() # default to OpenSSL on all other platforms
      if (NOT USE_MBED_TLS AND NOT USE_LIBRE_SSL) # Unless mbedtls or libressl is requested
        set(USE_OPEN_SSL ON)
      endif()
    endif()

    if (USE_MBED_TLS)
		if (WIN32)
			set(CACHE{MbedTLS_ROOT_DIR} TYPE PATH HELP "Install path of MbedTLS" VALUE "C:/Program Files (x86)/Mbed TLS")
		else()
			set(CACHE{MbedTLS_ROOT_DIR} TYPE PATH HELP "Install path of MbedTLS" VALUE)
		endif()
		set(MbedTLS_DIR "${MbedTLS_ROOT_DIR}/lib/cmake/MbedTLS/")
		find_package(MbedTLS REQUIRED)
		get_target_property(MBEDTLS_INCLUDE_DIRS MbedTLS::mbedtls INTERFACE_INCLUDE_DIRECTORIES)
		find_library(MBEDTLS_LIBRARY
			NAMES
				libmbedtls
				NAMES_PER_DIR
			PATHS ${MbedTLS_ROOT_DIR}
			PATH_SUFFIXES
				lib
		)
		find_library(MBEDX509_LIBRARY
			NAMES
				libmbedx509
				NAMES_PER_DIR
			PATHS ${MbedTLS_ROOT_DIR}
			PATH_SUFFIXES
				lib
		)
		find_library(MBEDCRYPTO_LIBRARY
			NAMES
				libtfpsacrypto
				NAMES_PER_DIR
			PATHS ${MbedTLS_ROOT_DIR}
			PATH_SUFFIXES
				lib
		)
		list(APPEND 
			OPT_LIB 
				MbedTLS::mbedtls
				MbedTLS::mbedx509
				MbedTLS::tfpsacrypto
		)
		list(APPEND 
			OPT_DEF
				M12_USE_MBEDTLS
		)
    elseif (USE_OPEN_SSL)
		if (WIN32)
			set(CACHE{OPENSSL_ROOT_DIR} TYPE PATH HELP "Install path of OpenSSL" VALUE "C:/Program Files (x86)/OpenSSL")
		else()
			set(CACHE{OPENSSL_ROOT_DIR} TYPE PATH HELP "Install path of OpenSSL" VALUE)
		endif()
		find_package(OpenSSL REQUIRED)
		list(APPEND 
			OPT_DEF
				M12_USE_OPENSSL
		)
	elseif (USE_LIBRE_SSL)
		if (WIN32)
			set(CACHE{LIBRESSL_ROOT_DIR} TYPE PATH HELP "Install path of LibreSSL" VALUE "C:/Program Files (x86)/LibreSSL")
		else()
			set(CACHE{LIBRESSL_ROOT_DIR} TYPE PATH HELP "Install path of LibreSSL" VALUE)
		endif()
		find_package(LibreSSL REQUIRED)
		list(APPEND 
			OPT_LIB 
				LibreSSL::TLS
				LibreSSL::SSL
				LibreSSL::Crypto
		)
		list(APPEND 
			OPT_DEF
				M12_USE_LIBRESSL
		)
    else()
        message(FATAL_ERROR "TLS Configuration error: unknown backend")
    endif()
endif()



if (USE_EXISTING_IXWEBSOCKET_LIB)
	if (WIN32)
		set(CACHE{IXWEBSOCKET_ROOT_DIR} TYPE PATH HELP "Install path of IXWebSocket (compiled with USE_ZLIB=0, USE_TLS=1 if compiled with wss support and the same TLS library)" VALUE "C:/Program Files (x86)/ixwebsocket")
	else()
		set(CACHE{IXWEBSOCKET_ROOT_DIR} TYPE PATH HELP "Install path of IXWebSocket (compiled with USE_ZLIB=0, USE_TLS=1 if compiled with wss support and the same TLS library)" VALUE)
	endif()
	find_package(IXWebSocket REQUIRED)
else()
	set(USE_ZLIB 0)
	#set(USE_TLS 1)
	#set(USE_LIBRE_SSL 1)
	set(USE_TEST 0)
	set(BUILD_SHARED_LIBS Off)
	set(IXWEBSOCKET_INSTALL Off)
	FetchContent_Declare(
		IXWebSocket
		GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
		CMAKE_ARGS -static
	)
	FetchContent_MakeAvailable(IXWebSocket)
endif()

if (WIN32)
	set(CACHE{GLFW3_ROOT_DIR} TYPE PATH HELP "Install path of Glfw3" VALUE "C:/Program Files (x86)/GLFW")
else()
	set(CACHE{GLFW3_ROOT_DIR} TYPE PATH HELP "Install path of Glfw3" VALUE)
endif()
find_package(Glfw3 REQUIRED)
#GLFW3 package can be found at https://www.glfw.org/
#TODO: get / compile GLFW automatically



if (WIN32)
	set(CMAKE_RC_COMPILER windres)
	enable_language(RC)
	add_library(wicon OBJECT)
	set_target_properties(wicon PROPERTIES LINKER_LANGUAGE CXX)
	target_sources(wicon
		PRIVATE
			include/icon/app.rc
	)
	list(APPEND 
		OPT_LIB 
			wicon
	)
endif()





if (WIN32)
	list(APPEND 
		OPT_LIB
			crypt32
			bcrypt
			gdi32
			Ws2_32
	)
endif()





target_compile_options(miniaudio PRIVATE -static -msse2 -mavx2)

target_compile_definitions(miniaudio
	PRIVATE
		MA_NO_DECODING
		MA_NO_ENCODING
		MA_NO_RESOURCE_MANAGER
		MA_NO_NODE_GRAPH
		
		${OPT_DEF}
)

target_sources(miniaudio
	PRIVATE
		src/miniaudio/miniaudio.c
	
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS
		include/miniaudio
	FILES
		include/miniaudio/miniaudio.h
)



target_compile_options(cjson PRIVATE -static)

target_compile_definitions(cjson
	PRIVATE
		CJSON_HIDE_SYMBOLS
		
		${OPT_DEF}
)

target_sources(cjson
	PRIVATE
		src/cJSON/cJSON.c
	
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS
		include/cJSON
	FILES
		include/cJSON/cJSON.h
)



target_compile_options(glloader PRIVATE -static)

target_compile_definitions(glloader
	PRIVATE
		${OPT_DEF}
)

target_sources(glloader
	PRIVATE
		src/glad/glad.c
	
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS
		include
	FILES
		include/glad/glad.h
		
		include/KHR/khrplatform.h
)



target_compile_options(imgui PRIVATE -static)

target_link_libraries(imgui
	PRIVATE
		glfw3
)

target_compile_definitions(imgui
	PRIVATE
		${OPT_DEF}
)

target_sources(imgui
	PRIVATE
		src/imgui/imgui.cpp
		src/imgui/imgui_demo.cpp#not needed
		src/imgui/imgui_draw.cpp
		src/imgui/imgui_tables.cpp
		src/imgui/imgui_widgets.cpp
		src/imgui/backends/imgui_impl_glfw.cpp
		src/imgui/backends/imgui_impl_opengl3.cpp
	
	PUBLIC
	FILE_SET HEADERS
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
)



target_compile_options(m12 PRIVATE -static -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers -msse2 -mavx2)

target_link_libraries(m12
	PRIVATE
		miniaudio
		cjson
		glloader
		imgui
		
		ixwebsocket
		
		glfw3
		
		stdc++
		m
		
		${OPT_LIB}
)

target_compile_definitions(m12
	PRIVATE
		_GLIBCXX_DEBUG
		_GLIBXX_DEBUG_PEDANTIC
		
		${OPT_DEF}
)

target_sources(m12
	PRIVATE
		src/main.cpp
		
		src/main_video.cpp
		
		src/data_path.cpp
		src/FileAccess.cpp
		
		src/main_circuit.cpp
		
		src/encoding.cpp
		src/thread_messaging.cpp
		
		src/circuit/80C32/80C32.cpp
		src/circuit/CPLD/MBSL_4000FH5_5.cpp
		src/circuit/ROM_256k/ROM_256k.cpp
		src/circuit/SRAM_64k/SRAM_64k.cpp
		src/circuit/TS9347/TS9347.cpp
	
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS
		include
	FILES
		include/data_path.h
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
		
		include/imgui/imgui.h
		include/imgui/imgui_impl_glfw.h
		include/imgui/imgui_impl_opengl3.h
		
		include/miniaudio/miniaudio.h
)





if(WIN32)
	get_target_property(M12_OUTPUT_NAME m12 OUTPUT_NAME)
	install(PROGRAMS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${M12_OUTPUT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
		DESTINATION "/"
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/TS9347/TS9347_Texture_Character_Set_Datasheet.bmp"
		DESTINATION "ressources"
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/ROM_256k/Bz6.rom"
		DESTINATION "rom"
	)
else()
	get_target_property(M12_OUTPUT_NAME m12 OUTPUT_NAME)
	install(PROGRAMS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${M12_OUTPUT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
		TYPE BIN
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/TS9347/TS9347_Texture_Character_Set_Datasheet.bmp"
		DESTINATION "com/M12/ressources"
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/ROM_256k/Bz6.rom"
		DESTINATION "com/M12/rom"
	)
endif()

include(CPack)
