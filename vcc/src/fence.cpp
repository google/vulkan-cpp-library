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
#include <vcc/fence.h>

namespace vcc {
namespace fence {

fence_type create(const type::supplier<device::device_type> &device,
		VkFenceCreateFlags flags) {
	VkFenceCreateInfo create = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL};
	create.flags = flags;
	VkFence fence;
	VKCHECK(vkCreateFence(internal::get_instance(*device), &create, NULL, &fence));
	return fence_type(fence, device);
}

VkResult wait(device::device_type &device,
		const std::vector<std::reference_wrapper<fence_type>> &fences,
		bool wait_all,
		std::chrono::nanoseconds timeout) {
	std::vector<VkFence> converted_fences;
	converted_fences.reserve(fences.size());
	for (const std::reference_wrapper<fence_type> &fence : fences) {
		converted_fences.push_back(internal::get_instance(fence.get()));
	}
	VkResult result = vkWaitForFences(internal::get_instance(device),
		(uint32_t) fences.size(), converted_fences.data(), !!wait_all,
		timeout.count());
	if (result != VK_TIMEOUT && result != VK_SUCCESS) {
		VKCHECK(result);
	}
	return result;
}

void reset(device::device_type &device,
	const std::vector<type::supplier<fence_type>> &fences) {
	std::vector<VkFence> converted_fences;
	converted_fences.reserve(fences.size());
	for (const type::supplier<fence_type> &fence : fences) {
		converted_fences.push_back(internal::get_instance(*fence));
	}
	std::vector<std::unique_lock<std::mutex>> locks;
	locks.reserve(fences.size());
	for (const type::supplier<fence_type> &fence : fences) {
		locks.emplace_back(internal::get_mutex(*fence), std::defer_lock);
	}
	util::lock(locks);
	VKCHECK(vkResetFences(internal::get_instance(device), uint32_t(fences.size()),
		converted_fences.data()));
}

}  // namespace fence
}  // namespace vcc
