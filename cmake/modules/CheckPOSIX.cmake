cmake_minimum_required(VERSION 3.10)

# ------------------------------------------------------------------------------
# Module:       CheckPOSIX
# Description:  Check for POSIX compliance
# ------------------------------------------------------------------------------
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCSourceCompiles)

set(POSIX_COMPLIANT FALSE)
set(CMAKE_REQUIRED_DEFINITIONS_ORIG ${CMAKE_REQUIRED_DEFINITIONS})
set(CMAKE_REQUIRED_LIBRARIES_ORIG ${CMAKE_REQUIRED_LIBRARIES})
set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} 
    -D_POSIX_C_SOURCE=200112L)
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} c)

check_include_file("unistd.h" HAVE_UNISTD_H)

# More cross-platform way of checking posix compliance
if(HAVE_UNISTD_H)
    check_c_source_compiles("
        #include <unistd.h>
        #include <sys/types.h>
        int main() {
            #ifdef _WIN32
                return 1; /* fork not available on standard Windows */
            #else
                pid_t pid = fork();
                return 0;
            #endif
        }
    " HAVE_FORK)
else()
    set(HAVE_FORK FALSE)
endif()

# Restore original flags
set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS_ORIG})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_ORIG})

# Determine POSIX compliance
if(HAVE_UNISTD_H AND HAVE_FORK)
    set(POSIX_COMPLIANT TRUE)
    message(STATUS "POSIX compliant system detected.")
else()
    message(STATUS "Non-POSIX compliant system detected: \
    unistd.h=${HAVE_UNISTD_H}, fork=${HAVE_FORK}")
    
    if(WIN32)
        message(STATUS "Windows platform detected, using alternative APIs.")
    endif()
endif()

set(POSIX_COMPLIANT ${POSIX_COMPLIANT} CACHE BOOL 
    "Is this a POSIX-compliant system?" FORCE)