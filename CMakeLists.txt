cmake_minimum_required (VERSION 3.21)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

project (cpp-smpp LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_compile_options(
    $<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall> # enable all warnings
    $<IF:$<CXX_COMPILER_ID:MSVC>,/permissive-,-fno-permissive> # disable language extensions by compilers
)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Set a default build type if none was specified (we are not using an IDE)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Add src before libs to make use it don't use installed headers
include_directories(src)

# Find Boost library
set(Boost_USE_STATIC_LIBS OFF) # Or we get errors with -fPIC
# set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS date_time)

# Google flags
find_package(gflags CONFIG REQUIRED)

# Google log
find_package(glog CONFIG REQUIRED)

set(link_libs
  Boost::boost
  Boost::date_time
  glog::glog
  gflags::gflags
)

add_subdirectory (src)

option (ENABLE_TEST "Compile unit testing" ON)

if (ENABLE_TEST)
  enable_testing()
  find_package(GTest CONFIG REQUIRED)
  add_subdirectory (test)
endif (ENABLE_TEST)
