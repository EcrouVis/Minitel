cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0207 NEW)#path separator
cmake_policy(SET CMP0135 NEW)#FetchContent timestamp
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_COLOR_DIAGNOSTICS On)
set(CMAKE_COLOR_MAKEFILE On)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS On)
#set(CMAKE_CXX_COMPILER g++)#TODO: works with gcc and clang but not msvc
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED On)
set(CMAKE_C_EXTENSIONS On)
#set(CMAKE_C_COMPILER gcc)

project(M12 VERSION 0.1.6.0 LANGUAGES CXX C)

include(FetchContent)
include(GNUInstallDirs)


if(CMAKE_BUILD_TYPE_INITIALIZED_TO_DEFAULT)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install/" CACHE PATH "Force path to set CMAKE_INSTALL_PREFIX" FORCE)
#endif()
#set(FETCHCONTENT_FULLY_DISCONNECTED On)






add_executable(m12)
set_target_properties(m12 PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}")
add_library(miniaudio STATIC)
add_library(cjson STATIC)
add_library(glloader STATIC)
add_library(imgui STATIC)





set(OPT_LIB )
set(OPT_DEF )
set(OPT_OPT )





option(USE_TLS "Use TLS for secure websockets" On)
option(USE_OPEN_SSL "Use OpenSSL" Off)
option(USE_LIBRE_SSL "Use LibreSSL" Off)
option(USE_MBED_TLS "Use MbedTLS" Off)
option(USE_DECOMP_TOOLS "Whether to compile with tools to help decompile the ROM" Off)
option(USE_SSE2 "Whether to use SSE2 for audio computation" On)
option(USE_AVX2 "Whether to use AVX2 for audio computation" On)
option(USE_EXISTING_IXWEBSOCKET_LIB "Use existing IXWebSocket library" Off)
option(USE_EXISTING_GLFW_LIB "Use existing GLFW library" Off)
if (LINUX)
	option(APPIMAGE_BUILD "Build for an AppImage" On)
endif()
if (WIN32)
	option(PORTABLE_BUILD "Build a portable executable" On)
endif()





if (USE_SSE2)
	list(APPEND 
		OPT_OPT
			-msse2
	)
endif()

if (USE_AVX2)
	list(APPEND 
		OPT_OPT
			-mavx2
	)
endif()

if (USE_DECOMP_TOOLS)
	list(APPEND 
		OPT_DEF
			M12_USE_DECOMP_TOOLS
	)
endif()

if (LINUX AND APPIMAGE_BUILD)
	list(APPEND 
		OPT_DEF
			M12_APPIMAGE
	)
endif()

if (WIN32 AND NOT PORTABLE_BUILD)
	list(APPEND 
		OPT_DEF
			M12_INSTALLED
	)
endif()

#get libs

#copied from IXWebsocket
if (USE_TLS)
	list(APPEND 
			OPT_DEF
				M12_USE_TLS
		)
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
		find_path(MBEDTLS_VERSION_GREATER_THAN_3 mbedtls/build_info.h PATHS ${MBEDTLS_INCLUDE_DIRS})
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
			OPT_LIB
				OpenSSL::SSL
				OpenSSL::Crypto
		)
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
		GIT_TAG master
	)
	#GIT_TAG -> master to use MbedTLS under windows (v12.0.1 does not work)
	FetchContent_MakeAvailable(IXWebSocket)
endif()

if (USE_EXISTING_GLFW_LIB)
	if (WIN32)
		set(CACHE{GLFW3_ROOT_DIR} TYPE PATH HELP "Install path of Glfw3" VALUE "C:/Program Files (x86)/GLFW")
	else()
		set(CACHE{GLFW3_ROOT_DIR} TYPE PATH HELP "Install path of Glfw3" VALUE)
	endif()
	find_package(Glfw3 REQUIRED)
else()
	set(GLFW_LIBRARY_TYPE STATIC)
	set(GLFW_BUILD_EXAMPLES Off)
	set(GLFW_BUILD_TESTS Off)
	set(GLFW_BUILD_DOCS Off)
	FetchContent_Declare(Glfw3
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(Glfw3)
endif()



if (WIN32)
	configure_file(src/desktop/app.rc.in src/desktop/app.rc)#cleaner that way if built in source
	file(COPY_FILE src/icon/M12_icon_square.ico "${CMAKE_CURRENT_BINARY_DIR}/src/desktop/M12_icon_square.ico")#preferred compared to generating the file path to the icon in app.rc because of string encoding of .rc files
	file(COPY_FILE src/desktop/M12.manifest "${CMAKE_CURRENT_BINARY_DIR}/src/desktop/M12.manifest")
	
	set(CMAKE_RC_COMPILER windres)
	enable_language(RC)
	
	add_library(wicon OBJECT)
	set_target_properties(wicon PROPERTIES LINKER_LANGUAGE CXX)
	target_sources(wicon
		PRIVATE
			"${CMAKE_CURRENT_BINARY_DIR}/src/desktop/app.rc"
	)
	
	list(APPEND 
		OPT_LIB 
			wicon
			
			dwmapi
	)
elseif(LINUX)
	configure_file(src/desktop/app.appimage.desktop.in "src/desktop/${CMAKE_PROJECT_NAME}.desktop")
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




target_compile_options(miniaudio PRIVATE ${OPT_OPT})

target_compile_definitions(miniaudio
	PRIVATE
		MA_NO_DECODING
		MA_NO_ENCODING
		MA_NO_RESOURCE_MANAGER
		MA_NO_NODE_GRAPH
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



target_compile_definitions(cjson
	PRIVATE
		CJSON_HIDE_SYMBOLS
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



target_link_libraries(imgui
	PRIVATE
		glfw
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



list(APPEND 
	OPT_OPT
		$<$<CONFIG:Release>:-flto>
)

target_compile_options(m12 PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers  ${OPT_OPT})

target_link_libraries(m12
	PRIVATE
		miniaudio
		cjson
		glloader
		imgui
		ixwebsocket
		glfw
		
		m
		stdc++
		pthread
		
		${OPT_LIB}
)

target_compile_definitions(m12
	PRIVATE
		_GLIBCXX_DEBUG
		_GLIBXX_DEBUG_PEDANTIC
		
		M12_VERSION="${CMAKE_PROJECT_VERSION}"
		
		${OPT_DEF}
)

target_sources(m12
	PRIVATE
		src/main.cpp
		
		src/main_video.cpp
		
		src/desktop/data_path.cpp
		src/FileAccess.cpp
		src/desktop/window_creation.cpp
		
		src/main_circuit.cpp
		
		src/encoding.cpp
		src/thread_messaging.cpp
		src/desktop/thread_affinity.cpp
		
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
		include/desktop/data_path.h
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
		include/desktop/thread_affinity.h
		include/desktop/window_creation.h
		
		include/circuit/80C32.h
		include/circuit/AudioBuffer.h
		include/circuit/BuzzerFilter.h
		include/circuit/clocks.h
		include/circuit/CPLD.h
		include/circuit/CRTBuffer.h
		include/circuit/Keyboard.h
		include/circuit/Latch.h
		include/circuit/PhoneLine.h
		include/circuit/PhoneLineBuffer.h
		include/circuit/PhoneLineWirring.h
		include/circuit/ROM_256k.h
		include/circuit/SpeakerFilter.h
		include/circuit/SRAM_64k.h
		include/circuit/TS7514.h
		include/circuit/TS9347.h
		include/circuit/WatchdogTimer.h
		include/circuit/debug/dbg_80C32.h
		include/circuit/debug/dbg_TS9347.h
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
		include/io/AudioIO.h
		
		include/icon/M12_icon_square.png.h
		
		include/stb_image.h
		include/stb_image_write.h
		
		include/glad/glad.h
		
		include/imgui/imgui.h
		include/imgui/imgui_impl_glfw.h
		include/imgui/imgui_impl_opengl3.h
)





if(WIN32)
	install(IMPORTED_RUNTIME_ARTIFACTS m12 
		RUNTIME_DEPENDENCY_SET _dependency_set 
		DESTINATION "/"
	)
	#TODO: rewrite more flexible (if mingw we could use: gcc -print-file-name=libwinpthread-1.dll to have the full path)
	#TODO: rewrite for cross compiling
	install(
		RUNTIME_DEPENDENCY_SET _dependency_set
		DIRECTORIES "C:/mingw64/x86_64-w64-mingw32/lib" "C:/llvm-mingw/x86_64-w64-mingw32/bin"
		PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
		POST_EXCLUDE_REGEXES  ".*system32/.*\\.dll" ".*SysWOW64/.*\\.dll"
		UNRESOLVED_DEPENDENCIES_VAR UNRES
		RUNTIME DESTINATION "/"
	)
	install(CODE [[
		foreach(UR ${UNRES})
			message(WARNING "${UR} dependency not resolved !")
		endforeach()
	]])
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/TS9347/TS9347_Texture_Character_Set_Datasheet.bmp"
		DESTINATION "data/ressources"
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/ROM_256k/Bz6.rom"
		DESTINATION "data/rom"
	)
	
	set(CPACK_GENERATOR ZIP NSIS)
	
	#NSIS specific
	set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_PROJECT_NAME}.exe")
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
	list(APPEND CPACK_NSIS_CREATE_ICONS_EXTRA "\
CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${CMAKE_PROJECT_NAME}.lnk' '$INSTDIR\\\\${CMAKE_PROJECT_NAME}.exe'\n\
CreateShortCut '$DESKTOP\\\\${CMAKE_PROJECT_NAME}.lnk' '$INSTDIR\\\\${CMAKE_PROJECT_NAME}.exe'\
	")
	list(APPEND CPACK_NSIS_DELETE_ICONS_EXTRA "\
Delete '$SMPROGRAMS\\\\$START_MENU\\\\${CMAKE_PROJECT_NAME}.lnk'\n\
Delete '$DESKTOP\\\\${CMAKE_PROJECT_NAME}.lnk'\
	")
elseif(LINUX)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/icon/M12_icon_square.svg"
		DESTINATION "usr/share/icons/hicolor/scalable/apps"
		RENAME "${CMAKE_PROJECT_NAME}.svg"
	)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/desktop/${CMAKE_PROJECT_NAME}.desktop"
		DESTINATION "usr/share/applications"
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/desktop/${CMAKE_PROJECT_NAME}.appdata.xml"
		DESTINATION "usr/share/metainfo"
	)
	install(IMPORTED_RUNTIME_ARTIFACTS m12
		DESTINATION "usr/bin"
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/TS9347/TS9347_Texture_Character_Set_Datasheet.bmp"
		DESTINATION "usr/share/M12/ressources"
	)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/circuit/ROM_256k/Bz6.rom"
		DESTINATION "usr/share/M12/rom"
	)
	
	set(CPACK_GENERATOR AppImage)
	
	set(CPACK_PACKAGE_ICON "${CMAKE_PROJECT_NAME}.svg")
	set(CPACK_APPIMAGE_TOOL_EXECUTABLE /home/yves/appimagetool-x86_64.AppImage)
endif()

set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_VENDOR "")
SET(CPACK_OUTPUT_FILE_PREFIX ./out)
include(CPack)
