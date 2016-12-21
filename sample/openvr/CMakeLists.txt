#
# Copyright 2016 Google Inc. All Rights Reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include_directories(include)
include_directories(../../types/include)
include_directories(../../vcc/include)
include_directories(${CMAKE_BINARY_DIR}/vcc/include)
include_directories(../../vcc-image/include)
include_directories(${FREEGLUT_INCLUDE_DIR})
include_directories(${GLM_SRC_DIR})
include_directories(${OPENVR_SOURCE_DIR}/headers)
if(NOT GLEW_BINARY_DIR STREQUAL "")
  include_directories(${GLEW_BINARY_DIR}/include)
  if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
      link_directories(${GLEW_BINARY_DIR}/lib/Release/x64)
    else()
      link_directories(${GLEW_BINARY_DIR}/lib/Release/Win32)
    endif()
  endif()
endif()
if(NOT VULKAN_SDK_DIR STREQUAL "")
  include_directories(${VULKAN_SDK_DIR}/include)
  link_directories(${VULKAN_SDK_DIR}/bin)
endif()

set(SAMPLE_OPENVR_INCLUDES
  "include/async_cache.h"
  "include/gl_objects.h"
  "include/glut_object.h"
  "include/thread_pool.h"
  "include/vr.h"
)

set(SAMPLE_OPENVR_SRCS
  "src/gl_objects.cpp"
  "src/glut.cpp"
  "src/openvr.cpp"
  "src/vr.cpp"
)

set(SAMPLE_OPENVR_SHADER_SRCS
  "src/openvr-frag.frag"
  "src/openvr-vert.vert"
)

if(UNIX)
  link_directories(${OPENVR_SOURCE_DIR}/lib/linux64)
elseif(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(${OPENVR_SOURCE_DIR}/lib/win64)
  else()
    link_directories(${OPENVR_SOURCE_DIR}/lib/win32)
  endif()
endif()

add_executable(openvr ${SAMPLE_OPENVR_INCLUDES} ${SAMPLE_OPENVR_SRCS})
target_link_libraries(openvr vcc vcc-image openvr_api ${GLEW_LIBRARY} freeglut_static)

foreach(FILE ${SAMPLE_OPENVR_SHADER_SRCS})
  get_filename_component(FILEWE ${FILE} NAME_WE)
  set(FILE_OUTPUT ${FILEWE}.spv)
  add_custom_command(TARGET openvr POST_BUILD DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
    COMMAND glslangValidator
    ARGS -V -o ${CMAKE_CURRENT_BINARY_DIR}/${FILE_OUTPUT} ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
    WORKING_DIRECTORY .)
endforeach()

add_custom_command(TARGET openvr POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:openvr>/textures)

