cmake_minimum_required(VERSION 3.17)

# ------------------------------------------------------------------------------
# Project Definition
# ------------------------------------------------------------------------------
project(AmberLauncher
    VERSION 1.0
    DESCRIPTION "Amber Island Launcher"
)
set(PROJECT_TITLE_SAFE 		"AmberIsland")
set(PROJECT_TITLE_FOLDER 	"Amber Island")
set(PROJECT_TITLE 			"Might and Magic 7: Amber Island")
set(PROJECT_EXE_NAME		"Amber Launcher")

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(AL_STATIC_LINKING    "Link libraries statically"                 OFF)
option(AL_BUILD_DOCS        "Build documentation with Doxygen"          OFF)
option(AL_BUILD_TESTS       "Build tests with CTest"                    OFF)

# ------------------------------------------------------------------------------
# C Standard Configuration
# ------------------------------------------------------------------------------
set(CMAKE_C_STANDARD            99)
set(CMAKE_C_STANDARD_REQUIRED   YES)
set(CMAKE_C_EXTENSIONS          NO)

set(CMAKE_SCRIPT_PATH ${CMAKE_SCRIPT_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# ------------------------------------------------------------------------------
# Flags
# ------------------------------------------------------------------------------
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DSYSTEM_ARCH_BITNESS=64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DSYSTEM_ARCH_BITNESS=32)
else()
    message(FATAL_ERROR 
    "Architecture bitness is not 64/32 bit.  \
     Define SYSTEM_ARCH_BITNESS=<YOURNUMBER>.")
endif()

# POSIX compliance
include(${CMAKE_MODULE_PATH}/CheckPOSIX.cmake)
if(POSIX_COMPLIANT)
    add_definitions(-D_POSIX_ENABLED)
endif()

# ------------------------------------------------------------------------------
# Debug
# ------------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)

    # libunwind (backtracing features)
    include(${CMAKE_MODULE_PATH}/CheckLibUnwind.cmake)
    if(LIBUNWIND_ENABLED)
        add_definitions(-D_LIBUNWIND_ENABLED)
        message(STATUS "Found libunwind")
    endif()
endif()

# ------------------------------------------------------------------------------
# MSVC
# ------------------------------------------------------------------------------
# Static Linking Configuration for MSVC
if(AL_STATIC_LINKING AND MSVC)
    # Set the runtime library to static
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ------------------------------------------------------------------------------
# MinGW
# ------------------------------------------------------------------------------
# Detect MinGW Cross Compilation
set(IS_MINGW FALSE)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(IS_MINGW TRUE)
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
add_subdirectory(deps)

# ------------------------------------------------------------------------------
# Integrate nappgui
# ------------------------------------------------------------------------------
find_package(nappgui REQUIRED)

include("${NAPPGUI_ROOT_PATH}/prj/NAppProject.cmake")
include("${NAPPGUI_ROOT_PATH}/prj/NAppCompilers.cmake")
nap_config_compiler()

Message(STATUS "[AL] NAPPGUI INC:  ${NAPPGUI_INCLUDE_PATH}")
Message(STATUS "[AL] NAPPGUI LIB:  ${NAPPGUI_LIBRARIES}")
Message(STATUS "[AL] NAPPGUI ROOT: ${NAPPGUI_ROOT_PATH}")
Message(STATUS "[AL] NAPPGUI NRC:  ${NAPPGUI_NRC}")
Message(STATUS "[AL] NAPPGUI PRJ:  ${NAPPGUI_PRJ_PATH}")

# ------------------------------------------------------------------------------
# Source Files
# ------------------------------------------------------------------------------
file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/src/AmberLauncherCore/*.c")

# ------------------------------------------------------------------------------
# Executable Target
# ------------------------------------------------------------------------------
add_library(AmberLauncher ${SRC_FILES})
nap_project_desktop_app(AmberLauncherGUI src/AmberLauncherGUI)
nap_target_c_standard(AmberLauncherGUI 90)
#nap_target_cxx_standard(AmberLauncherGUI 11)
nap_link_inet(AmberLauncherGUI)

# ------------------------------------------------------------------------------
# Include Directories
# ------------------------------------------------------------------------------
target_include_directories(AmberLauncher PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
    ${NAPPGUI_INCLUDE_PATH}
)

# ------------------------------------------------------------------------------
# Link Libraries
# ------------------------------------------------------------------------------
target_link_libraries(AmberLauncher PRIVATE 
    ${LUA_LIBRARIES}
)

# ------------------------------------------------------------------------------
# Compiler Properties
# ------------------------------------------------------------------------------
set_target_properties(AmberLauncherGUI PROPERTIES
    OUTPUT_NAME "Amber Launcher"
)

if(MSVC)
    set_target_properties(AmberLauncherGUI PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
	set_target_properties(AmberLauncherGUI PROPERTIES
		LINK_FLAGS "/MANIFEST:NO /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
	)
endif()
# Set working directory to source dir for debugging in MSVC

# ------------------------------------------------------------------------------
# Compiler Flags
# ------------------------------------------------------------------------------
# Set default compiler flags based on compiler type
if(MSVC)
    set(CMAKE_C_FLAGS "/W3 /WX /permissive- /std:c99")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	target_compile_options(AmberLauncherGUI PRIVATE
        /wd4061
		/wd4127
    )
else()
    # Base flags for all build types
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -std=c99")
    
    # Debug-specific compiler flags for GCC/Clang
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} \
            -Wstrict-overflow -Wconversion \
            -Wdouble-promotion \
            -Wnull-dereference \
            -Wno-sign-conversion \
            -Wshadow \
            -Wpointer-arith \
            -Wno-newline-eof \
            -Wstrict-prototypes \
            -Wmissing-prototypes \
            -fPIC \
            -g \
            -fPIE")
        
        if(UNIX AND NOT APPLE)
            set(CMAKE_C_FLAGS_DEBUG
                "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize-recover=address"
                CACHE STRING "C flags for Debug" FORCE)
            
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG
                "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -g3"
                CACHE STRING "Linker flags for Debug" FORCE)
        endif()
endif()

# ------------------------------------------------------------------------------
# Debug-Specific Link Settings
# ------------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
    # Linux-specific debug link dependencies
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries(AmberLauncher PRIVATE unwind)
    endif()
elseif(WIN32)
    # Windows-specific debug link dependencies
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries(AmberLauncher PRIVATE dbghelp)
    endif()
endif()

# ------------------------------------------------------------------------------
# MinGW Specific Settings
# ------------------------------------------------------------------------------
if(IS_MINGW)
    # Define a macro for MinGW
    add_definitions(-DUSE_MINGW)
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} <whatever>")
endif()

# ------------------------------------------------------------------------------
# Doxygen Documentation Generation
# ------------------------------------------------------------------------------
if(AL_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    if(DOXYGEN_FOUND)
        # Configure Doxygen
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs)
        set(DOXYGEN_INPUT_DIR 
            ${CMAKE_SOURCE_DIR}/include 
            ${CMAKE_SOURCE_DIR}/src)

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
if(AL_BUILD_TESTS)
    # @todo
    enable_testing()
    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------------------
# Installation preparation
# ------------------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS AmberLauncherGUI
    RUNTIME DESTINATION .)

install(FILES
    ${CMAKE_SOURCE_DIR}/icon.ico
    ${CMAKE_SOURCE_DIR}/LICENSE
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/manifest.json
    DESTINATION .)

# Extra folders
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data     DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Scripts  DESTINATION .)

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
# NappGUI Manifest hack (MSVC)
# ------------------------------------------------------------------------------
if(MSVC)
    set(MANIFEST_FILE "${CMAKE_SOURCE_DIR}/src/AmberLauncherGUI/res/Amber.manifest")
    set(RES_FILE "${CMAKE_CURRENT_BINARY_DIR}/src/AmberLauncherGUI/res.rc")

    file(READ "${RES_FILE}" file_contents)
    string(REPLACE "\r\n" "\n" file_contents "${file_contents}")
    string(REPLACE "\r" "\n" file_contents "${file_contents}")
    string(REGEX REPLACE "\n$" "" file_contents "${file_contents}")

    string(REPLACE "\n" ";" file_lines "${file_contents}")

    list(LENGTH file_lines line_count)

    set(new_line "1 24 \"${MANIFEST_FILE}\"")

    if(line_count GREATER 1)
        # Replace existing line 2
        list(REMOVE_AT file_lines 1)
        list(INSERT file_lines 1 "${new_line}")
    else()
        # Append a second line
        list(APPEND file_lines "${new_line}")
    endif()

    string(REPLACE ";" "\n" modified_content "${file_lines}")
    file(WRITE "${RES_FILE}" "${modified_content}")

    message(STATUS "[AL] Manifest inserted/updated in: ${RES_FILE}")
    message(STATUS "[AL] Manifest used: ${MANIFEST_FILE}")
endif()

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
#                     "$<TARGET_RUNTIME_DLLS:AmberLauncher>" 
#                       USES_TERMINAL COMMAND_EXPAND_LISTS
#         )
#endif()

# ------------------------------------------------------------------------------
# Packaging with CPack
# ------------------------------------------------------------------------------
set(CPACK_ARCHIVE_ZIP_FILE_NAME      	"${PROJECT_TITLE_SAFE}_${PROJECT_VERSION}_win64")
set(CPACK_ARCHIVE_TGZ_FILE_NAME      	"${PROJECT_TITLE_SAFE}_${PROJECT_VERSION}_linux")
set(CPACK_NSIS_PACKAGE_NAME          	"${PROJECT_TITLE}")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY 	".")
set(CPACK_NSIS_EXECUTABLES			 	"${PROJECT_EXE_NAME}.exe")
set(CPACK_NSIS_MUI_FINISHPAGE_RUN 	 	"${PROJECT_EXE_NAME}.exe")
set(CPACK_PACKAGE_NAME               	"${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION            	"${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR             	"Henrik Chukhran")
set(CPACK_PACKAGE_CONTACT            	"dev@mightandmagicmod.com")

#set(CPACK_NSIS_MUI_HEADERIMAGE 			"${PROJECT_SOURCE_DIR}/Data/Launcher/nsis_header.bmp")
#set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_SOURCE_DIR}/Data/Launcher/nsis_welcome.bmp")

# generators per-platform
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_TITLE_FOLDER}")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

set(CPACK_RESOURCE_FILE_LICENSE      	"${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README       	"${PROJECT_SOURCE_DIR}/README.md")

include(CPack)
