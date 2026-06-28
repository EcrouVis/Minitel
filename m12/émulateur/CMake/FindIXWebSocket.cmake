INCLUDE(FindPackageHandleStandardArgs)

set(_IXWEBSOCKET_ROOT_HINTS
    ${IXWEBSOCKET_ROOT_DIR}
    ENV IXWEBSOCKET_ROOT_DIR
)

if (WIN32)
    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _programfiles)
    set(_IXWEBSOCKET_ROOT_PATHS
        "${_programfiles}/ixwebsocket"
    )
    unset(_programfiles)
elseif(APPLE)
    set(_IXWEBSOCKET_ROOT_PATHS
        "/usr/local/opt/ixwebsocket"
    )
else()
    set(_IXWEBSOCKET_ROOT_PATHS
        "/usr/local/"
    )
endif()

set(_IXWEBSOCKET_ROOT_HINTS_AND_PATHS
    HINTS ${_IXWEBSOCKET_ROOT_HINTS}
    PATHS ${_IXWEBSOCKET_ROOT_PATHS}
)

find_path(IXWEBSOCKET_INCLUDE_DIR
    NAMES
        ixwebsocket/IXWebSocket.h
    ${_IXWEBSOCKET_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES
        include
)

find_library(IXWEBSOCKET_LIBRARY
    NAMES
        libixwebsocket
		ixwebsocket
        NAMES_PER_DIR
    ${_IXWEBSOCKET_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES
        lib
)

mark_as_advanced(IXWEBSOCKET_INCLUDE_DIR IXWEBSOCKET_LIBRARY)

find_package_handle_standard_args(IXWebSocket
    REQUIRED_VARS
        IXWEBSOCKET_LIBRARY
        IXWEBSOCKET_INCLUDE_DIR
    HANDLE_COMPONENTS
        FAIL_MESSAGE
        "Could NOT find IXWebSocket, try setting the path to IXWebSocket using the IXWEBSOCKET_ROOT_DIR environment variable"
)

if(IXWEBSOCKET_FOUND)

    if(NOT TARGET ixwebsocket AND EXISTS "${IXWEBSOCKET_LIBRARY}")

        add_library(ixwebsocket UNKNOWN IMPORTED)

        set_target_properties(
            ixwebsocket
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${IXWEBSOCKET_INCLUDE_DIR}"
                IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                IMPORTED_LOCATION "${IXWEBSOCKET_LIBRARY}"
        )

    endif()

endif(IXWEBSOCKET_FOUND)
