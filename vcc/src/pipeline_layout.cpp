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
#include <vcc/pipeline_layout.h>

namespace vcc {
namespace pipeline_layout {
namespace internal {

vcc::internal::hook_container_type<queue::queue_type &> &get_pre_execute_callbacks(pipeline_layout_type &layout) {
	return layout.pre_execute_callbacks;
}

}  // namespace internal

pipeline_layout_type create(const type::supplier<device::device_type> &device,
		const std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> &set_layouts,
		const std::vector<VkPushConstantRange> &push_constant_ranges) {
	VkPipelineLayoutCreateInfo create = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, NULL, 0};
	create.setLayoutCount = (uint32_t) set_layouts.size();
	std::vector<VkDescriptorSetLayout> converted_set_layouts;
	converted_set_layouts.reserve(set_layouts.size());
	std::transform(set_layouts.begin(), set_layouts.end(), std::back_inserter(converted_set_layouts),
			[](const type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type> &set_layout){
		return vcc::internal::get_instance(*set_layout);
	});
	create.pSetLayouts = set_layouts.empty() ? NULL : &converted_set_layouts.front();
	create.pushConstantRangeCount = (uint32_t) push_constant_ranges.size();
	create.pPushConstantRanges = push_constant_ranges.empty() ? NULL : &push_constant_ranges.front();
	VkPipelineLayout pipeline_layout;
	VKCHECK(vkCreatePipelineLayout(vcc::internal::get_instance(*device), &create, NULL, &pipeline_layout));
	return pipeline_layout_type(pipeline_layout, device);
}

}  // namespace pipeline_layout
}  // namespace vcc


