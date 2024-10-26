# This CMake file allows compiling ping-cpp from submodule

cmake_minimum_required(VERSION 3.12)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

project(evologics_driver)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")


find_package(Protobuf REQUIRED)
find_package(goby 3.1 REQUIRED)


add_library(evologics_driver SHARED
  src/evologics_driver/evologics_driver.cpp
  src/AT/HayesAtDecoder.cpp
  src/AT/HayesAtEncoder.cpp
)

target_include_directories(evologics_driver PUBLIC
    src/AT 
    src/evologics_driver
)

target_link_libraries(evologics_driver LINK_PUBLIC
  goby
)