cmake_minimum_required(VERSION 3.19)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")

project(Handao LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory for .exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "C:/path/to/devtools/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Help CMake find Tesseract and Leptonica
list(APPEND CMAKE_PREFIX_PATH "C:/path/to/devtools/vcpkg/installed/x64-windows-static")

# Find Tesseract and Leptonica
find_package(Tesseract REQUIRED)
find_package(Leptonica REQUIRED)
find_package(SFML COMPONENTS Graphics Window REQUIRED)

# Static linking for MSVC runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add source files
add_subdirectory(src)