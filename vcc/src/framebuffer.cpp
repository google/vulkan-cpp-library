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
#include <algorithm>
#include <iterator>
#include <vcc/framebuffer.h>

namespace vcc {
namespace framebuffer {

framebuffer_type create(const type::supplier<device::device_type> &device,
	const type::supplier<render_pass::render_pass_type> &render_pass,
	const std::vector<type::supplier<image_view::image_view_type>> &image_views,
	VkExtent2D extent, uint32_t layers) {

	VkFramebufferCreateInfo create = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, NULL, 0 };
	create.renderPass = internal::get_instance(*render_pass);
	create.attachmentCount = (uint32_t) image_views.size();
	std::vector<VkImageView> converted_image_views;
	converted_image_views.reserve(image_views.size());
	std::transform(image_views.begin(), image_views.end(),
		std::back_inserter(converted_image_views),
		[](const type::supplier<image_view::image_view_type> &image_view) {
		return internal::get_instance(*image_view);
	});
	create.pAttachments = &converted_image_views.front();
	create.width = extent.width;
	create.height = extent.height;
	create.layers = layers;
	VkFramebuffer framebuffer;
	VKCHECK(vkCreateFramebuffer(internal::get_instance(*device), &create, NULL, &framebuffer));
	return framebuffer_type(framebuffer, device, render_pass, image_views);
}

}  // namespace framebuffer
}  // namespace vcc
