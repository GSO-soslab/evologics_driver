# Locate and configure the Google Protocol Buffers library.
# Defines the following variables:
#
#   PROTOBUF_FOUND - Found the Google Protocol Buffers library
#   PROTOBUF_INCLUDE_DIRS - Include directories for Google Protocol Buffers
#   PROTOBUF_LIBRARIES - The protobuf library
#
# The following cache variables are also defined:
#   PROTOBUF_LIBRARY - The protobuf library
#   PROTOBUF_PROTOC_LIBRARY   - The protoc library
#   PROTOBUF_INCLUDE_DIR - The include directory for protocol buffers
#   PROTOBUF_PROTOC_EXECUTABLE - The protoc compiler
#
#  ====================================================================
#  Example:
#
#   find_package(ProtobufLocal REQUIRED)
#   include_directories(${PROTOBUF_INCLUDE_DIRS})
#
#   include_directories(${CMAKE_CURRENT_BINARY_DIR})
#   PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} foo.proto)
#   add_executable(bar bar.cc ${PROTO_SRCS} ${PROTO_HDRS})
#   target_link_libraries(bar ${PROTOBUF_LIBRARY})
#
# NOTE: You may need to link against pthreads, depending
# on the platform.
#  ====================================================================
#
# PROTOBUF_GENERATE_CPP (public function)
#   SRCS = Variable to define with autogenerated
#          source files
#   HDRS = Variable to define with autogenerated
#          header files
#   CPP_OUT_DIR = Directory to store the autogenerated source and header files (*.pb.h and *.pb.cc)
#   ARGN = proto files
#
#  ====================================================================
#
#  PROTOBUF_INCLUDE_DIRS (public function)
#   ARGN = Directories to add to the protoc (Google Protocol Buffer compiler) include path
#
#  ====================================================================


#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
# Copyright 2008 Esben Mose Hansen, Ange Optimization ApS
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

function(PROTOBUF_INCLUDE_DIRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_INCLUDE_DIRS() called without any directories")
    return()
  endif()  

  foreach(DIR ${ARGN})
    set(ALL_PROTOBUF_INCLUDE_DIRS "${ALL_PROTOBUF_INCLUDE_DIRS};-I${DIR}" PARENT_SCOPE)
  endforeach()
endfunction()

function(PROTOBUF_GENERATE_CPP SRCS HDRS CPP_OUT_DIR)
  protobuf_include_dirs(${CMAKE_CURRENT_BINARY_DIR})

  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif(NOT ARGN)

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    
    list(APPEND ${SRCS} "${CPP_OUT_DIR}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${CPP_OUT_DIR}/${FIL_WE}.pb.h")   
    message("CPP OUT DIR: ${CPP_OUT_DIR}/${FIL_WE}")
    add_custom_command(
      OUTPUT "${CPP_OUT_DIR}/${FIL_WE}.pb.cc"
             "${CPP_OUT_DIR}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --cpp_out ${CPP_OUT_DIR} ${ABS_FIL} "-I${CMAKE_CURRENT_SOURCE_DIR}" ${ALL_PROTOBUF_INCLUDE_DIRS} --dccl_out ${CPP_OUT_DIR}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)

  include_directories(${CPP_OUT_DIR})

endfunction()


find_path(PROTOBUF_INCLUDE_DIR google/protobuf/service.h)

# so that we can use Google's included descriptor.proto
list(APPEND ALL_PROTOBUF_INCLUDE_DIRS "-I${PROTOBUF_INCLUDE_DIR}")


# Google's provided vcproj files generate libraries with a "lib"
# prefix on Windows
if(WIN32)
    set(PROTOBUF_ORIG_FIND_LIBRARY_PREFIXES "${CMAKE_FIND_LIBRARY_PREFIXES}")
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
endif()

find_library(PROTOBUF_LIBRARY NAMES protobuf
             DOC "The Google Protocol Buffers Library"
)
find_library(PROTOBUF_PROTOC_LIBRARY NAMES protoc
             DOC "The Google Protocol Buffers Compiler Library"
)
find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc
             DOC "The Google Protocol Buffers Compiler"
)

mark_as_advanced(PROTOBUF_INCLUDE_DIR
                 PROTOBUF_LIBRARY
                 PROTOBUF_PROTOC_LIBRARY
                 PROTOBUF_PROTOC_EXECUTABLE)

# Restore original find library prefixes
if(WIN32)
    set(CMAKE_FIND_LIBRARY_PREFIXES "${PROTOBUF_ORIG_FIND_LIBRARY_PREFIXES}")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROTOBUF DEFAULT_MSG
    PROTOBUF_LIBRARY PROTOBUF_INCLUDE_DIR)

if(PROTOBUF_FOUND)
    set(PROTOBUF_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIR})
    set(PROTOBUF_LIBRARIES    ${PROTOBUF_LIBRARY})

    execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --version
      OUTPUT_VARIABLE PROTOC_VERSION_STRING)
    string(REPLACE "libprotoc "
      "" PROTOC_VERSION ${PROTOC_VERSION_STRING})

endif()