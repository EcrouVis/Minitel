INCLUDE(FindPackageHandleStandardArgs)

set(_GLFW3_ROOT_HINTS
    ${GLFW3_ROOT_DIR}
    ENV GLFW3_ROOT_DIR
)

if (WIN32)
    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _programfiles)
    set(_GLFW3_ROOT_PATHS
        "${_programfiles}/GLFW"
    )
    unset(_programfiles)
elseif(APPLE)
    set(_GLFW3_ROOT_PATHS
        "/usr/local/opt/glfw"
    )
else()
    set(_GLFW3_ROOT_PATHS
        "/usr/local/"
    )
endif()

set(_GLFW3_ROOT_HINTS_AND_PATHS
    HINTS ${_GLFW3_ROOT_HINTS}
    PATHS ${_GLFW3_ROOT_PATHS}
)

find_path(GLFW3_INCLUDE_DIR
    NAMES
        GLFW/glfw3.h
    ${_GLFW3_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES
        include
)

find_library(GLFW3_LIBRARY
    NAMES
        libglfw3
		glfw3
        NAMES_PER_DIR
    ${_GLFW3_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES
        lib
)

mark_as_advanced(GLFW3_INCLUDE_DIR GLFW3_LIBRARY)

find_package_handle_standard_args(Glfw3
    REQUIRED_VARS
        GLFW3_LIBRARY
        GLFW3_INCLUDE_DIR
    HANDLE_COMPONENTS
        FAIL_MESSAGE
        "Could NOT find Glfw3, try setting the path to Glfw3 using the GLFW3_ROOT_DIR environment variable"
)

if(GLFW3_FOUND)

    if(NOT TARGET glfw3 AND EXISTS "${GLFW3_LIBRARY}")

        add_library(glfw3 UNKNOWN IMPORTED)

        set_target_properties(
            glfw3
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${GLFW3_INCLUDE_DIR}"
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${GLFW3_LIBRARY}"
        )

    endif()

endif(GLFW3_FOUND)
