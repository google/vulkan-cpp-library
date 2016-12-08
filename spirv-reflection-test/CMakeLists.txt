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

include_directories(../spirv-reflection/include)
include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${SPIRV-Headers_SOURCE_DIR}/include/)
include_directories(${SPIRV_TOOLS_SRC}/include/)

set(SPIRV_REFLECTION_TEST_SRCS
  "src/subpass_test.cpp"
  "src/push_constant_test.cpp"
  "src/uniform_buffer_test.cpp"
  "src/specialization_constant_test.cpp"
  "src/input_test.cpp"
)

set(SPIRV_REFLECTION_TEST_SHADER_SRCS
  "src/subpass_test1.frag"
  "src/push_constant_test1.comp"
  "src/uniform_buffer_test1.comp"
  "src/specialization_constant_test1.comp"
  "src/input_test1.vert"
)

add_executable(spirv-reflection-test ${SPIRV_REFLECTION_TEST_SRCS})
target_link_libraries(spirv-reflection-test spirv-reflection SPIRV-Tools gtest gtest_main)

set(SPIRV_REFLECTION_TEST_COMPILED_SHADER_BINARIES)
foreach(FILE ${SPIRV_REFLECTION_TEST_SHADER_SRCS})
  get_filename_component(FILEWE ${FILE} NAME_WE)
  set(FILE_OUTPUT ${FILEWE}.spv)
  add_custom_command(TARGET spirv-reflection-test POST_BUILD DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
    COMMAND glslangValidator
    ARGS -V -o ${CMAKE_CURRENT_BINARY_DIR}/${FILE_OUTPUT} ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
    WORKING_DIRECTORY .)
  list(APPEND SPIRV_REFLECTION_TEST_COMPILED_SHADER_BINARIES ${FILE_OUTPUT})
endforeach()

add_test(spirv-reflection-tests spirv-reflection-test)

