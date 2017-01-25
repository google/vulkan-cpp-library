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
include_directories(${GLM_SRC_DIR})
include_directories(include)

set(TYPES_INCLUDES
  "include/type/storage.h"
  "include/type/serialize.h"
  "include/type/types.h"
  "include/type/internal.h"
  "include/type/transform.h"
  "include/type/memory.h"
  "include/type/revision.h"
  "include/type/supplier.h"
)

set(TYPES_SRCS
  "src/memory.cpp"
  "src/serialize.cpp"
)

add_library(types ${TYPES_INCLUDES} ${TYPES_SRCS})

