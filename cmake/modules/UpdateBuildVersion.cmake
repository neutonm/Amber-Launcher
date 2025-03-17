# Update build number

cmake_minimum_required(VERSION 3.10)

function(update_build_number)
    if(EXISTS ${VERSION_FILE})
        file(READ ${VERSION_FILE} VERSION_CONTENT)
        string(REGEX MATCH "BUILD_NUMBER \"([0-9]+)\"" _ ${VERSION_CONTENT} BUILD_NUMBER)
        math(EXPR BUILD_NUMBER "${CMAKE_MATCH_1} + 1")
        message("Build number: ${BUILD_NUMBER}")
    else()
        set(BUILD_NUMBER 1)
    endif()

    file(WRITE ${VERSION_FILE} "#ifndef VERSION_H_\n#define VERSION_H_\n\n#define BUILD_NUMBER \"${BUILD_NUMBER}\"\n\n#endif\n")
endfunction()

update_build_number()
