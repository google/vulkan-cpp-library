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
#include <vcc/buffer_view.h>

namespace vcc {
namespace buffer_view {

buffer_view_type create(const type::supplier<const buffer::buffer_type> &buffer,
		VkFormat format, VkDeviceSize offset, VkDeviceSize range) {
	VkBufferViewCreateInfo create = {VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO, NULL, 0};
	create.buffer = vcc::internal::get_instance(*buffer);
	create.format = format;
	create.offset = offset;
	create.range = range;
	VkBufferView view;
	VKCHECK(vkCreateBufferView(internal::get_instance(*internal::get_parent(*buffer)),
		&create, NULL, &view));
	return buffer_view_type(view, internal::get_parent(*buffer), buffer);
}

}  // namespace buffer_view
}  // namespace vcc
