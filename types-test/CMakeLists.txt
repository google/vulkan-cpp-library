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

include_directories(../types/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(TYPES_TEST_SRCS
  "src/merge_type_test.cpp"
  "src/serialize_type_test.cpp"
  "src/transform_type_test.cpp"
  "src/view_type_test.cpp"
  "src/storage_type_test.cpp"
)

add_executable(types-test ${TYPES_TEST_SRCS})
target_link_libraries(types-test types gtest gtest_main)

add_test(types-tests types-test)

