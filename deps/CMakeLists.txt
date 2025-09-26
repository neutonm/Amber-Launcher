include(FetchContent)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(AL_USE_BUNDLED_LUA
       "Download & build Lua in-tree instead of using an installed copy"
       OFF)
option(AL_USE_BUNDLED_NAPPGUI
       "Download & build NappGUI in-tree instead of using an installed copy"
       OFF)

if(MSVC)
    set(AL_USE_BUNDLED_LUA ON CACHE BOOL "Download & build Lua in-tree for MSVC" FORCE)
    set(AL_USE_BUNDLED_NAPPGUI ON CACHE BOOL "Download & build NappGUI in-tree for MSVC" FORCE)
endif()

# ------------------------------------------------------------------------------
# Lua
# ------------------------------------------------------------------------------

if(NOT AL_USE_BUNDLED_LUA)
    find_package(Lua REQUIRED )
endif()
if (Lua_FOUND)
    message("Lua found...")
    set(LUA_LIBRARIES ${LUA_LIBRARIES} PARENT_SCOPE)
    set(LUA_INCLUDE_DIR ${LUA_INCLUDE_DIR} PARENT_SCOPE)
else()
    # Fetch Lua from the official repository
    FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/lua/lua.git
        GIT_TAG v5.4.8
        OVERRIDE_FIND_PACKAGE
    )
    add_subdirectory(lua)
    set(LUA_INCLUDE_DIR "${LUA_INCLUDE_DIR}" PARENT_SCOPE)
    set(LUA_LIBRARIES "${LUA_LIBRARIES}" PARENT_SCOPE)
endif()

# ------------------------------------------------------------------------------
# NappGUI
# ------------------------------------------------------------------------------

if(AL_USE_BUNDLED_NAPPGUI)
    message("Fetching NappGUI...")
    FetchContent_Declare(
        nappgui_src
        GIT_REPOSITORY https://github.com/frang75/nappgui_src.git
        GIT_TAG v1.5.2
    )
    FetchContent_Populate(nappgui_src)

    # Where to build & install NAppGUI
    set(NAPPGUI_BUILD_DIR ${CMAKE_BINARY_DIR}/_nappgui_build)
    set(NAPPGUI_INSTALL_DIR ${CMAKE_BINARY_DIR}/_nappgui_install)

    # Configure + build + install NAppGUI
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S ${nappgui_src_SOURCE_DIR}
            -B ${NAPPGUI_BUILD_DIR}
            -DCMAKE_INSTALL_PREFIX=${NAPPGUI_INSTALL_DIR}
            -DNAPPGUI_DEMO=OFF
            -G "${CMAKE_GENERATOR}"
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to configure NAppGUI")
    endif()

    # Multi-config (MSVC, Xcode) vs single-config
    get_property(isMulti GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(isMulti)
        set(NAPPGUI_BUILD_CONFIG "Release")
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${NAPPGUI_BUILD_DIR} --target install --config ${NAPPGUI_BUILD_CONFIG}
            RESULT_VARIABLE result
        )
    else()
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${NAPPGUI_BUILD_DIR} --target install --target install
            RESULT_VARIABLE result
        )
    endif()
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to build NAppGUI")
    endif()

    list(PREPEND CMAKE_PREFIX_PATH "${NAPPGUI_INSTALL_DIR}/cmake")
endif()

find_package(nappgui REQUIRED)

set(NAPPGUI_INCLUDE_PATH ${NAPPGUI_INCLUDE_PATH} PARENT_SCOPE)
set(NAPPGUI_LIBRARIES ${NAPPGUI_LIBRARIES} PARENT_SCOPE)
set(NAPPGUI_ROOT_PATH ${NAPPGUI_ROOT_PATH} PARENT_SCOPE)
set(NAPPGUI_NRC ${NAPPGUI_NRC} PARENT_SCOPE)
