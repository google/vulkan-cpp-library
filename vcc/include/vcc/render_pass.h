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
#ifndef RENDER_PASS_H_
#define RENDER_PASS_H_

#include <vcc/device.h>

namespace vcc {
namespace render_pass {

struct subpass_description_type {
	std::vector<VkAttachmentReference> inputAttachments;
	std::vector<VkAttachmentReference> colorAttachments;
	std::vector<VkAttachmentReference> resolveAttachments;
	VkAttachmentReference depthStencilAttachment;
	std::vector<uint32_t> preserveAttachments;
};

struct render_pass_type
	: public internal::movable_destructible_with_parent<VkRenderPass,
	device::device_type, vkDestroyRenderPass> {
	friend VCC_LIBRARY render_pass_type create(
		const type::supplier<device::device_type> &device,
		const std::vector<VkAttachmentDescription> &attachment_descriptions,
		const std::vector<subpass_description_type> &subpass_descriptions,
		const std::vector<VkSubpassDependency> &subpass_dependency);

	render_pass_type() = default;
	render_pass_type(render_pass_type &&) = default;
	render_pass_type(const render_pass_type &) = delete;
	render_pass_type &operator=(render_pass_type &&) = default;
	render_pass_type &operator=(const render_pass_type &) = delete;

private:
	render_pass_type(VkRenderPass instance,
		const type::supplier<device::device_type> &parent)
		: internal::movable_destructible_with_parent<VkRenderPass,
		device::device_type, vkDestroyRenderPass>(instance, parent) {}
};

VCC_LIBRARY render_pass_type create(
	const type::supplier<device::device_type> &device,
	const std::vector<VkAttachmentDescription> &attachment_descriptions,
	const std::vector<subpass_description_type> &subpass_descriptions,
	const std::vector<VkSubpassDependency> &subpass_dependency);

VCC_LIBRARY VkExtent2D get_render_area_granularity(const render_pass_type &render_pass);

}  // namespace render_pass
}  // namespace vcc

#endif /* RENDER_PASS_H_ */
