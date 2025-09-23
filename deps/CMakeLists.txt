include(FetchContent)

# newer cmake versions should automatically call find_package at FetchContent_Declare

option(AL_USE_BUNDLED_LUA
       "Download & build Lua in-tree instead of using an installed copy"
       OFF)

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


