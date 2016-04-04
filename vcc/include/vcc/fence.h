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
#ifndef FENCE_H_
#define FENCE_H_

#include <chrono>
#include <vcc/device.h>
#include <vcc/fence.h>

namespace vcc {
namespace fence {

struct fence_type
	: public internal::movable_destructible_with_parent<VkFence,
	device::device_type, vkDestroyFence> {
	friend VCC_LIBRARY fence_type create(
		const type::supplier<device::device_type> &device, VkFenceCreateFlags flags);

	fence_type() = default;
	fence_type(fence_type &&) = default;
	fence_type(const fence_type &) = delete;
	fence_type &operator=(fence_type &&) = default;
	fence_type &operator=(const fence_type &) = delete;

private:
	fence_type(VkFence instance,
		const type::supplier<device::device_type> &parent)
		: internal::movable_destructible_with_parent<VkFence,
		device::device_type, vkDestroyFence>(instance, parent) {}
};

VCC_LIBRARY fence_type create(
	const type::supplier<device::device_type> &device,
	VkFenceCreateFlags flags = 0);

VCC_LIBRARY VkResult wait(device::device_type &device,
		const std::vector<std::reference_wrapper<fence_type>> &fences,
		bool wait_all,
		std::chrono::nanoseconds timeout);

VCC_LIBRARY void reset(device::device_type &device,
	const std::vector<type::supplier<fence_type>> &fences);

}  // namespace fence
}  // namespace vcc

#endif /* FENCE_H_ */
