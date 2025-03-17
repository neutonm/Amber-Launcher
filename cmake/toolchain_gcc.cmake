# Use GCC as the compiler
set(CMAKE_C_COMPILER   gcc)
set(CMAKE_CXX_COMPILER g++)

# Toolchain must be specified before the `project()` call.
set(CMAKE_SYSTEM_NAME Generic) 
