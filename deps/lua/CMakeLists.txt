message(STATUS "Fetching Lua...")

FetchContent_MakeAvailable(lua)

# Detect the major / minor version in lua.h
if(EXISTS "${lua_SOURCE_DIR}/lua.h")
    set(_lua_h "${lua_SOURCE_DIR}/lua.h")
elseif(EXISTS "${lua_SOURCE_DIR}/src/lua.h")
    set(_lua_h "${lua_SOURCE_DIR}/src/lua.h")
else()
    message(FATAL_ERROR "lua.h not found under ${lua_SOURCE_DIR}")
endif()

file(READ "${_lua_h}" _lua_h_content)

# Extract the decimal value after '#define LUA_VERSION_NUM'
string(REGEX MATCH
       "#[ \t]*define[ \t]+LUA_VERSION_NUM[ \t]+([0-9]+)"
       _ver_match "${_lua_h_content}")
set(LUA_VERSION_NUM "${CMAKE_MATCH_1}")

if(LUA_VERSION_NUM STREQUAL "")
    message(FATAL_ERROR "Could not parse LUA_VERSION_NUM in ${_lua_h}")
endif()

message(STATUS "Detected Lua version num = ${LUA_VERSION_NUM}")

# List of Lua source files (adjust according to the Lua version)
set(LUA_SOURCES
    ${lua_SOURCE_DIR}/lapi.c
    ${lua_SOURCE_DIR}/lcode.c
    #${lua_SOURCE_DIR}/lctype.c
    ${lua_SOURCE_DIR}/ldebug.c
    ${lua_SOURCE_DIR}/ldo.c
    ${lua_SOURCE_DIR}/ldump.c
    ${lua_SOURCE_DIR}/lfunc.c
    ${lua_SOURCE_DIR}/lgc.c
    ${lua_SOURCE_DIR}/llex.c
    ${lua_SOURCE_DIR}/lmem.c
    ${lua_SOURCE_DIR}/loadlib.c
    ${lua_SOURCE_DIR}/lobject.c
    ${lua_SOURCE_DIR}/lopcodes.c
    ${lua_SOURCE_DIR}/lparser.c
    ${lua_SOURCE_DIR}/lstate.c
    ${lua_SOURCE_DIR}/lstring.c
    ${lua_SOURCE_DIR}/ltable.c
    ${lua_SOURCE_DIR}/ltm.c
    ${lua_SOURCE_DIR}/lundump.c
    ${lua_SOURCE_DIR}/lvm.c
    ${lua_SOURCE_DIR}/lzio.c
    ${lua_SOURCE_DIR}/lauxlib.c
    ${lua_SOURCE_DIR}/lbaselib.c
    #${lua_SOURCE_DIR}/lcorolib.c
    ${lua_SOURCE_DIR}/ldblib.c
    ${lua_SOURCE_DIR}/liolib.c
    ${lua_SOURCE_DIR}/lmathlib.c
    ${lua_SOURCE_DIR}/loslib.c
    ${lua_SOURCE_DIR}/lstrlib.c
    ${lua_SOURCE_DIR}/ltablib.c
    ${lua_SOURCE_DIR}/linit.c
    #${lua_SOURCE_DIR}/lutf8lib.c
)

if(LUA_VERSION_NUM GREATER 501)
    list(APPEND LUA_SOURCES
        ${lua_SOURCE_DIR}/lctype.c
        ${lua_SOURCE_DIR}/lcorolib.c
    )
endif()
if(LUA_VERSION_NUM GREATER 502)
    list(APPEND LUA_SOURCES
        ${lua_SOURCE_DIR}/lutf8lib.c
    )
endif()
if(LUA_VERSION_NUM EQUAL 502 OR LUA_VERSION_NUM EQUAL 503)
    list(APPEND LUA_SOURCES
        ${lua_SOURCE_DIR}/lbitlib.c
    )
endif()

if(MSVC)
    foreach(flag_var
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_MINSIZEREL
    )
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
endif()

add_library(lua_static STATIC ${LUA_SOURCES})

# Compiler flags
set_target_properties(lua_static PROPERTIES
    COMPILE_OPTIONS ""
    COMPILE_DEFINITIONS ""
)

target_include_directories(lua_static PUBLIC ${lua_SOURCE_DIR})

# Set the C standard to C89 for the Lua target
set_target_properties(lua_static PROPERTIES
    C_STANDARD 90
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)
target_compile_definitions(lua_static PRIVATE LUA_USE_C89)

# Adjust compiler flags as specified in the Lua makefile
if(MSVC)
    target_compile_options(lua_static PRIVATE
        $<$<CONFIG:Debug>:/W3>
        $<$<CONFIG:Release>:/O2 /W3>
    )
else()
    target_compile_options(lua_static PRIVATE
        -O2
        -Wall
        -Wextra
    )
endif()

# Platform-specific definitions and libraries
if(WIN32)
    target_compile_definitions(lua_static PRIVATE LUA_BUILD_AS_DLL LUA_USE_WINDOWS)
else()
    target_compile_definitions(lua_static PRIVATE LUA_USE_POSIX LUA_USE_DLOPEN LUA_USE_READLINE)
    target_link_libraries(lua_static PRIVATE dl m readline)
endif()

# Suppress specific warnings for the lua_static target
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(lua_static PRIVATE
        -Wno-implicit-fallthrough
        -Wno-null-dereference
        -Wno-cast-align
        -Wno-sign-conversion
        -Wno-implicit-int-conversion
        -Wno-implicit-function-declaration
        -Wno-invalid-noreturn
        -Wno-conversion
        -Wno-int-conversion
        -Wno-string-plus-int
        -Wno-error
        -Wno-address
        -Wno-misleading-indentation
    )
elseif(MSVC)
    target_compile_options(lua_static PRIVATE
        /wd4244  # Conversion from 'type1' to 'type2', possible loss of data
        /wd4267  # Conversion from 'size_t' to 'type', possible loss of data
        /wd4245  # Conversion from 'type1' to 'type2', signed/unsigned mismatch
        /wd4311  # Pointer truncation from 'type' to 'type'
        /wd4312  # Conversion from 'type' to 'type' of greater size
        /WX-     # Do not treat warnings as errors
    )
endif()

# Set variables for the parent scope
set(LUA_INCLUDE_DIR ${lua_SOURCE_DIR} PARENT_SCOPE)
set(LUA_LIBRARIES "lua_static" PARENT_SCOPE)
