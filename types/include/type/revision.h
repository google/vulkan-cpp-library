/*
* Copyright 2016 Google Inc. All Rights Reserved.

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef GTYPE_REVISION_TYPE_H_
#define GTYPE_REVISION_TYPE_H_

namespace type {

// Not using 64 bit, even with 100 updates/second it will take +1 year to overflow.
typedef unsigned long revision_type;
// If a container returns REVISION_NONE it doesn't use a revision system.
// An Adapter has its initial value set to REVISION_NONE to force an update.
// (the default revision is 1)
const revision_type REVISION_NONE = 0;

}  // namespace type

#endif // GTYPE_REVISION_TYPE_H_
