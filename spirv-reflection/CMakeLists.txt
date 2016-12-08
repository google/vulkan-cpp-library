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
include_directories(../types/include)
include_directories(${SPIRV-Headers_SOURCE_DIR}/include/)
include_directories(${SPIRV_TOOLS_SRC}/include/)

set(SPIRV_REFLECTION_INCLUDES
  "include/reflection/analyzer.h"
  "include/reflection/types.h"
  "include/reflection/internal/argument_parser.h"
  "include/reflection/internal/includes.h"
  "include/reflection/internal/intermediate_types.h"
)

set(SPIRV_REFLECTION_SRCS
  "src/analyzer.cpp"
)

add_library(spirv-reflection ${SPIRV_REFLECTION_INCLUDES} ${SPIRV_REFLECTION_SRCS})

