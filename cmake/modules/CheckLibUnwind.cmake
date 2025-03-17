cmake_minimum_required(VERSION 3.10)

# ------------------------------------------------------------------------------
# Module:       CheckLibUnwind
# Description:  Check for libunwind - backtrace library
# ------------------------------------------------------------------------------

project(CheckPOSIX)

include(CheckIncludeFile)
include(CheckSymbolExists)

check_include_file("libunwind.h" HAVE_LIBUNWIND_H)
check_symbol_exists(fork "libunwind.h" HAVE_FORK)

if(HAVE_LIBUNWIND_H AND HAVE_FORK)
    set(LIBUNWIND_ENABLED TRUE)
else()
    set(LIBUNWIND_ENABLED FALSE)
endif() 
