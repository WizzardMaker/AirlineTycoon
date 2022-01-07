# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

# Which compilers to use for C and C++
set(CMAKE_C_COMPILER gcc -m32)
set(CMAKE_CXX_COMPILER g++ -m32)

#https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_LIBRARY_ARCHITECTURE.html#variable:CMAKE_%3CLANG%3E_LIBRARY_ARCHITECTURE
set(CMAKE_LIBRARY_ARCHITECTURE i686)
