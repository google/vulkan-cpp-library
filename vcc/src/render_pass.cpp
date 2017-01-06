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
#include <vcc/render_pass.h>

namespace vcc {
namespace render_pass {

std::vector<VkSubpassDescription> subpass_descriptions_convert(
		const std::vector<subpass_description_type> &subpass_descriptions) {
	std::vector<VkSubpassDescription> descriptions;
	descriptions.reserve(subpass_descriptions.size());
	std::transform(subpass_descriptions.begin(), subpass_descriptions.end(),
			std::back_inserter(descriptions),
			[](const subpass_description_type &description) {
		VkSubpassDescription desc;
		desc.flags = 0;
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.inputAttachmentCount = (uint32_t) description.inputAttachments.size();
		desc.pInputAttachments = description.inputAttachments.data();
		desc.colorAttachmentCount = (uint32_t)description.colorAttachments.size();
		desc.pColorAttachments = description.colorAttachments.data();
		desc.pResolveAttachments = description.resolveAttachments.data();
		desc.pDepthStencilAttachment = &description.depthStencilAttachment;
		desc.preserveAttachmentCount = (uint32_t)description.preserveAttachments.size();
		desc.pPreserveAttachments = description.preserveAttachments.data();
		return desc;
	});
	return std::move(descriptions);
}

render_pass_type create(const type::supplier<const device::device_type> &device,
		const std::vector<VkAttachmentDescription> &attachment_descriptions,
		const std::vector<subpass_description_type> &subpass_descriptions,
		const std::vector<VkSubpassDependency> &subpass_dependency) {
	VkRenderPassCreateInfo create = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, NULL, 0};
	create.attachmentCount = (uint32_t)attachment_descriptions.size();
	create.pAttachments = attachment_descriptions.empty() ? NULL : &attachment_descriptions.front();
	create.subpassCount = (uint32_t)subpass_descriptions.size();
	const std::vector<VkSubpassDescription> subpass_descriptions_converted(
			subpass_descriptions_convert(subpass_descriptions));
	create.pSubpasses = subpass_descriptions_converted.empty() ? NULL : &subpass_descriptions_converted.front();
	create.dependencyCount = (uint32_t)subpass_dependency.size();
	create.pDependencies = subpass_dependency.empty() ? NULL : &subpass_dependency.front();
	VkRenderPass render_pass;
	VKCHECK(vkCreateRenderPass(internal::get_instance(*device), &create, NULL, &render_pass));
	return render_pass_type(render_pass, device);
}

VkExtent2D get_render_area_granularity(const render_pass_type &render_pass) {
	VkExtent2D extent;
	vkGetRenderAreaGranularity(
		internal::get_instance(*internal::get_parent(render_pass)),
		internal::get_instance(render_pass), &extent);
	return extent;
}

}  // namespace render_pass
}  // namespace vcc


