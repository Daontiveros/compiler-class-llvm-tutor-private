cmake_minimum_required(VERSION 3.20)
project(AffineRecurrence)

# Path to your LLVM installation
set(LLVM_DIR "/usr/lib/llvm-21/lib/cmake/llvm" CACHE PATH "LLVM installation directory")
list(APPEND CMAKE_PREFIX_PATH "${LLVM_DIR}")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake from ${LLVM_DIR}")

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 17)

# Build the plugin from the correct source file (AffineRecurrence.cpp)
add_library(AffineRecurrence SHARED AffineRecurrence.cpp)

# Link flags for macOS (ignored on Linux)
target_link_libraries(AffineRecurrence
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

# Make sure LLVM headers are found
target_include_directories(AffineRecurrence PRIVATE ${LLVM_INCLUDE_DIRS})
