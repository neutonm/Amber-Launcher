cmake_minimum_required(VERSION 3.10)

project(CheckPOSIX)

include(CheckIncludeFile)
include(CheckSymbolExists)

check_include_file("unistd.h" HAVE_UNISTD_H)
check_symbol_exists(fork "unistd.h" HAVE_FORK)

if(HAVE_UNISTD_H AND HAVE_FORK)
    set(POSIX_COMPLIANT TRUE)
else()
    set(POSIX_COMPLIANT FALSE)
endif() 
