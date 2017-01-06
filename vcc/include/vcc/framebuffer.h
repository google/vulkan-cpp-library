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
#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include <vcc/device.h>
#include <vcc/image_view.h>
#include <vcc/render_pass.h>

namespace vcc {
namespace framebuffer {

struct framebuffer_type : internal::movable_destructible_with_parent<VkFramebuffer,
		const device::device_type, vkDestroyFramebuffer> {
	friend VCC_LIBRARY framebuffer_type create(const type::supplier<const device::device_type> &,
		const type::supplier<const render_pass::render_pass_type> &,
		const std::vector<type::supplier<const image_view::image_view_type>> &, VkExtent2D,
		uint32_t);

	framebuffer_type() = default;
	framebuffer_type(framebuffer_type &&) = default;
	framebuffer_type(const framebuffer_type &) = delete;
	framebuffer_type &operator=(framebuffer_type &&) = default;
	framebuffer_type &operator=(const framebuffer_type &) = delete;

private:
	framebuffer_type(VkFramebuffer instance,
		const type::supplier<const device::device_type> &parent,
		const type::supplier<const render_pass::render_pass_type> &render_pass,
		const std::vector<type::supplier<const image_view::image_view_type>> &image_views)
		: movable_destructible_with_parent(instance, parent)
		, render_pass(render_pass), image_views(image_views) {}

	type::supplier<const render_pass::render_pass_type> render_pass;
	std::vector<type::supplier<const image_view::image_view_type>> image_views;
};

VCC_LIBRARY framebuffer_type create(
	const type::supplier<const device::device_type> &device,
	const type::supplier<const render_pass::render_pass_type> &render_pass,
	const std::vector<type::supplier<const image_view::image_view_type>> &image_views,
	VkExtent2D extent, uint32_t layers);

}  // namespace framebuffer
}  // namespace vcc

#endif /* FRAMEBUFFER_H_ */
