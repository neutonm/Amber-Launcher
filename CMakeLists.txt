cmake_minimum_required(VERSION 3.14)

# ------------------------------------------------------------------------------
# Project Definition
# ------------------------------------------------------------------------------
project(AmberLauncher
    VERSION 1.0
    DESCRIPTION "Amber Island Launcher"
    LANGUAGES C
)

# ------------------------------------------------------------------------------
# C Standard Configuration
# ------------------------------------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS NO)

set(CMAKE_SCRIPT_PATH ${CMAKE_SCRIPT_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(OPTION_STATIC_LINKING "Link libraries statically" OFF)
option(OPTION_BUILD_DOCS "Build documentation with Doxygen" OFF)
option(OPTION_BUILD_TESTS "Build tests with CTest" OFF)

# ------------------------------------------------------------------------------
# Flags
# ------------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_definitions(-DSYSTEM_ARCH_BITNESS=64)
else()
  add_definitions(-DSYSTEM_ARCH_BITNESS=32)
endif()

# POSIX compliance
#include(${CMAKE_MODULE_PATH}/CheckPOSIX.cmake)

if(POSIX_COMPLIANT)
    add_definitions(-D_POSIX_ENABLED)
    message(STATUS "System is POSIX compliant, _POSIX_ENABLED defined")
else()
    message(STATUS "System is not POSIX compliant")
endif()

# Lib Unwind
if(CMAKE_BUILD_TYPE MATCHES Debug)
  include(${CMAKE_MODULE_PATH}/CheckLibUnwind.cmake)
  if(LIBUNWIND_ENABLED)
      add_definitions(-D_LIBUNWIND_ENABLED)
      message(STATUS "Found libunwind")
  endif()
endif()

# ------------------------------------------------------------------------------
# Static Linking Configuration for MSVC
# ------------------------------------------------------------------------------
if(OPTION_STATIC_LINKING AND MSVC)
    # Set the runtime library to static
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ------------------------------------------------------------------------------
# Find Required Packages
# ------------------------------------------------------------------------------
# Removed all raylib-related configurations

# Find libcurl
find_package(CURL REQUIRED)
if(NOT CURL_FOUND)
    message(FATAL_ERROR "libcurl not found. Please install libcurl and ensure CMake can locate it.")
endif()

# Find Lua
find_package(Lua REQUIRED)
if(NOT Lua_FOUND)
    message(FATAL_ERROR "Lua not found. Please install Lua and ensure CMake can locate it.")
endif()

# ------------------------------------------------------------------------------
# Detect MinGW Cross Compilation
# ------------------------------------------------------------------------------
set(IS_MINGW FALSE)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(IS_MINGW TRUE)
endif()

# ------------------------------------------------------------------------------
# Integrate nappgui
# ------------------------------------------------------------------------------

# Optionally find a pre-installed nappgui
find_package(nappgui REQUIRED)
if(nappgui_FOUND)
    message(STATUS "Found pre-installed nappgui.")
else()
    message(FATAL_ERROR "nappgui not found. Please install nappgui or enable OPTION_USE_NAPPGUI_AUTOMATICALLY.")
endif()
include("${NAPPGUI_ROOT_PATH}/prj/NAppProject.cmake")
include("${NAPPGUI_ROOT_PATH}/prj/NAppCompilers.cmake")
nap_config_compiler()

Message("NAPPGUI INC: ${NAPPGUI_INCLUDE_PATH}")
Message("NAPPGUI LIB: ${NAPPGUI_LIBRARIES}")

# ------------------------------------------------------------------------------
# Source Files
# ------------------------------------------------------------------------------
file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/src/AmberLauncherCore/*.c")

# ------------------------------------------------------------------------------
# Executable Target
# ------------------------------------------------------------------------------
add_library(AmberLauncher ${SRC_FILES})
nap_project_desktop_app(AmberLauncherGUI src/AmberLauncherGUI)

# ------------------------------------------------------------------------------
# Include Directories
# ------------------------------------------------------------------------------
target_include_directories(AmberLauncher PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR}
    ${NAPPGUI_INCLUDE_PATH}
)

# ------------------------------------------------------------------------------
# Link Libraries
# ------------------------------------------------------------------------------
target_link_libraries(AmberLauncher PRIVATE 
    ${CURL_LIBRARIES} 
    ${LUA_LIBRARIES}
)

# ------------------------------------------------------------------------------
# Compiler Flags
# ------------------------------------------------------------------------------
if(MSVC)
    target_compile_options(AmberLauncher PRIVATE /W4 /WX /permissive- /std:c99)
else()
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_options(AmberLauncher PRIVATE 
            -Wstrict-overflow 
            -Wall 
            -Wextra 
            -Wpedantic 
            -Werror 
            -Wconversion 
            -Wdouble-promotion 
            -Wnull-dereference
            -Wno-sign-conversion
            -Wshadow 
            -Wpointer-arith 
            #-Wcast-align
            -Wno-newline-eof
            -Wstrict-prototypes 
            -Wmissing-prototypes 
            -fPIC
            -g
            -fPIE 
            #-fsanitize=address
        )
    else()
        target_compile_options(AmberLauncher PRIVATE 
            -Wall 
            -Wextra 
            -Wpedantic 
            -Werror 
            -std=c99)
    endif()
endif()

# ------------------------------------------------------------------------------
# Debug-Specific Settings
# ------------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
    # Linux specific debug settings
    # target_compile_options(AmberLauncher PRIVATE
    #     $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=leak -fsanitize-recover=address>
    # )
    # target_link_options(AmberLauncher PRIVATE
    #     $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -g3>
    # )
    target_link_libraries(AmberLauncher PRIVATE
        $<$<CONFIG:Debug>:unwind>
    )
elseif(WIN32)
    # Windows specific debug settings
    target_link_libraries(AmberLauncher PRIVATE
        $<$<CONFIG:Debug>:dbghelp>
    )
endif()

# ------------------------------------------------------------------------------
# MinGW Specific Settings
# ------------------------------------------------------------------------------
if(IS_MINGW)
    # Define a macro or add specific flags for MinGW if necessary
    target_compile_definitions(AmberLauncher PRIVATE USE_MINGW)
    # Add any MinGW-specific linker flags here
endif()

# ------------------------------------------------------------------------------
# Doxygen Documentation Generation
# ------------------------------------------------------------------------------
if(OPTION_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    if(DOXYGEN_FOUND)
        # Configure Doxygen
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs)
        set(DOXYGEN_INPUT_DIR ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

        # Create Doxyfile
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            "PROJECT_NAME           = Amber Island Launcher\n"
            "OUTPUT_DIRECTORY       = ${DOXYGEN_OUTPUT_DIR}\n"
            "INPUT                  = ${DOXYGEN_INPUT_DIR}\n"
            "RECURSIVE              = YES\n"
            "GENERATE_LATEX         = NO\n"
            "GENERATE_HTML          = YES\n"
            "EXTRACT_ALL            = YES\n"
        )

        # Add custom target for generating documentation
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
        message(WARNING "Doxygen not found. Documentation will not be built.")
    endif()
endif()

# ------------------------------------------------------------------------------
# Testing with CTest
# ------------------------------------------------------------------------------
if(OPTION_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------------------
# Automated Build Version
# ------------------------------------------------------------------------------
set(VERSION_FILE "${PROJECT_SOURCE_DIR}/include/version.h")

add_custom_command(
    TARGET AmberLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
        -DTARGET_NAME=AmberLauncher
        -DVERSION_FILE=${VERSION_FILE}
        -DTARGET_PATH=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_MODULE_PATH}/UpdateBuildVersion.cmake
    COMMENT "Running script..."
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# ------------------------------------------------------------------------------
# Copy EXE into project folder
# ------------------------------------------------------------------------------
# @todo Temporary
add_custom_command(TARGET AmberLauncher
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                           $<TARGET_FILE:AmberLauncher>
                           ${CMAKE_SOURCE_DIR})

# ------------------------------------------------------------------------------
# Copy DLLs into project folder
# ------------------------------------------------------------------------------
#if(WIN32)
# add_custom_command (
#             TARGET AmberLauncher POST_BUILD
#             COMMAND "${CMAKE_COMMAND}" -E copy -t "${CMAKE_SOURCE_DIR}"
#                     "$<TARGET_RUNTIME_DLLS:AmberLauncher>" USES_TERMINAL COMMAND_EXPAND_LISTS
#         )
#endif()
