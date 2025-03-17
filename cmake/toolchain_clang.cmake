# Use Clang as the compiler
set(CMAKE_C_COMPILER   clang)
set(CMAKE_CXX_COMPILER clang++)

# Toolchain must be specified before the `project()` call.
set(CMAKE_SYSTEM_NAME Generic) 
