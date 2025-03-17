
nap_desktop_app(AmberLauncherGUI "AmberLauncher" NRC_EMBEDDED)
target_include_directories(AmberLauncherGUI PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
Message("SOURCE DIR: ${CMAKE_SOURCE_DIR}")
add_custom_command(TARGET AmberLauncherGUI
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                           $<TARGET_FILE:AmberLauncherGUI>
                           ${CMAKE_SOURCE_DIR})