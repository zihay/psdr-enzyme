cmake_minimum_required(VERSION 3.15)

project(psdr-enzyme
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(LLVM_DIR "/usr/lib/llvm-12/lib/cmake/llvm/")

# Build the dependencies
add_subdirectory(ext)

# add_subdirectory(src)
add_subdirectory(gtests)