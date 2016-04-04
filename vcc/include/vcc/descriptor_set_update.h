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
#ifndef DESCRIPTOR_SET_UPDATE_H_
#define DESCRIPTOR_SET_UPDATE_H_

#include <vcc/descriptor_set.h>

namespace vcc {
namespace descriptor_set {

// takes device plus a list of copy and write_* items.
template <typename... ArgsT>
void update(device::device_type &device, const ArgsT &... args) {
	internal::update_storage storage;
	util::internal::pass( (internal::count(storage, args), 1)...);
	storage.reserve();
	util::internal::pass((internal::add(storage, args), 1)...);
	vcc::util::lock(storage.deferred_locks);
	VKTRACE(vkUpdateDescriptorSets(vcc::internal::get_instance(device),
		(uint32_t) storage.write_sets.size(), storage.write_sets.data(),
		(uint32_t) storage.copy_sets.size(), storage.copy_sets.data()));
}

}  // namespace descriptor_set
}  // namespace vcc

#endif /* DESCRIPTOR_SET_UPDATE_H_ */
