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
#ifndef BUFFER_VIEW_H_
#define BUFFER_VIEW_H_

#include <vcc/buffer.h>

namespace vcc {
namespace buffer_view {

struct buffer_view_type
	: public internal::movable_destructible_with_parent<VkBufferView,
		const device::device_type, vkDestroyBufferView> {
	friend VCC_LIBRARY buffer_view_type create(
		const type::supplier<const buffer::buffer_type> &buffer, VkFormat format,
		VkDeviceSize offset, VkDeviceSize range);

	buffer_view_type(buffer_view_type &&) = default;
	buffer_view_type(const buffer_view_type &) = default;
	buffer_view_type &operator=(buffer_view_type &&) = default;
	buffer_view_type &operator=(const buffer_view_type &) = delete;

private:
	buffer_view_type(VkBufferView instance,
		const type::supplier<const device::device_type> &parent,
		const type::supplier<const buffer::buffer_type> &buffer)
		: movable_destructible_with_parent(instance, parent),
		buffer(buffer) {}

	type::supplier<const buffer::buffer_type> buffer;
};

VCC_LIBRARY buffer_view_type create(
	const type::supplier<const buffer::buffer_type> &buffer, VkFormat format,
	VkDeviceSize offset, VkDeviceSize range);

}  // namespace buffer_view
}  // namespace vcc

#endif /* BUFFER_VIEW_H_ */
