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
#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <vcc/device.h>

namespace vcc {
namespace semaphore {

struct semaphore_type
	: public internal::movable_destructible_with_parent<VkSemaphore,
	device::device_type, vkDestroySemaphore> {
	friend VCC_LIBRARY semaphore_type create(
		const type::supplier<device::device_type> &device);

	semaphore_type() = default;
	semaphore_type(semaphore_type &&) = default;
	semaphore_type(const semaphore_type &) = delete;
	semaphore_type &operator=(semaphore_type &&) = default;
	semaphore_type &operator=(const semaphore_type &) = delete;

private:
	semaphore_type(VkSemaphore instance,
		const type::supplier<device::device_type> &parent)
		: internal::movable_destructible_with_parent<VkSemaphore,
		device::device_type, vkDestroySemaphore>(instance, parent) {}
};

VCC_LIBRARY semaphore_type create(
	const type::supplier<device::device_type> &device);

}  // namespace semaphore
}  // namespace vcc

#endif /* SEMAPHORE_H_ */
