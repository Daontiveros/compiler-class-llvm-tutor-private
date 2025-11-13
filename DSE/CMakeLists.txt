cmake_minimum_required(VERSION 3.20)
project(DeadStoreElimination)

# Path to your LLVM installation
set(LLVM_DIR "/usr/lib/llvm-21/lib/cmake/llvm" CACHE PATH "LLVM installation directory")
list(APPEND CMAKE_PREFIX_PATH "${LLVM_DIR}")

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake from ${LLVM_DIR}")

# Include LLVM headers and definitions
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build the LLVM plugin from DeadStoreElimination.cpp
add_library(DeadStoreElimination SHARED DeadStoreElimination.cpp)

# Link flags (macOS only — ignored on Linux)
target_link_libraries(DeadStoreElimination
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

# Ensure LLVM headers and definitions are visible
target_include_directories(DeadStoreElimination PRIVATE ${LLVM_INCLUDE_DIRS})