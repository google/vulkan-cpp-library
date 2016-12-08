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
#ifndef PIPELINE_H_
#define PIPELINE_H_

#include <type/serialize.h>
#include <vcc/pipeline_cache.h>
#include <vcc/pipeline_layout.h>
#include <vcc/render_pass.h>
#include <vcc/shader_module.h>
#include <unordered_map>

namespace vcc {
namespace pipeline {

struct shader_stage_type {
	VkShaderStageFlagBits stage;
	type::supplier<shader_module::shader_module_type> module;
	std::string name;
	std::vector<VkSpecializationMapEntry> map_entries;
	std::string data;
};

inline shader_stage_type shader_stage(VkShaderStageFlagBits stage,
		const type::supplier<shader_module::shader_module_type> &module,
		const std::string &name,
		const std::vector<VkSpecializationMapEntry> &map_entries = {},
		const std::string &data = std::string()) {
	return shader_stage_type{ stage, module, name, map_entries, data };
}

template<typename... StorageType>
shader_stage_type shader_stage(VkShaderStageFlagBits stage,
	const type::supplier<shader_module::shader_module_type> &module,
	const std::string &name,
	const std::vector<VkSpecializationMapEntry> &map_entries,
	StorageType... storages) {
	type::serialize_type serialize(type::make_serialize(
		type::memory_layout::linear, std::forward<StorageType>(storages)...));
	std::string data(type::size(serialize), '\0');
	type::flush(serialize, &data[0]);
	return shader_stage(stage, module, name, map_entries, data);
}

struct vertex_input_state {
	std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
};

struct input_assembly_state {
	VkPrimitiveTopology topology;
	VkBool32 primitiveRestartEnable;
};

struct tessellation_state {
	uint32_t patchControlPoints;
};

struct viewport_state_type {
	// Either viewport_count = viewports.size
	// or viewports.empty
	// which only works if viewports are a dynamic state.
	uint32_t viewport_count;
	std::vector<VkViewport> viewports;
	// Either scissor_count = scissors.size
	// or scissors.empty
	// which only works if viewports are a dynamic state.
	uint32_t scissor_count;
	std::vector<VkRect2D> scissors;
};

inline viewport_state_type viewport_state(uint32_t viewport_count, uint32_t scissor_count) {
	return viewport_state_type{ viewport_count, {}, scissor_count };
}

inline viewport_state_type viewport_state(
		const std::vector<VkViewport> &viewports,
		const std::vector<VkRect2D> &scissors) {
	return viewport_state_type{ uint32_t(viewports.size()), viewports,
		uint32_t(scissors.size()), scissors };
}

inline viewport_state_type viewport_state(
	const std::vector<VkViewport> &viewports, uint32_t scissor_count) {
	return viewport_state_type{ uint32_t(viewports.size()), viewports,
		scissor_count };
}

inline viewport_state_type viewport_state(
	uint32_t viewport_count,
	const std::vector<VkRect2D> &scissors) {
	return viewport_state_type{ viewport_count, {}, uint32_t(scissors.size()),
		scissors };
}

struct rasterization_state {
	VkBool32 depthClampEnable;
	VkBool32 rasterizerDiscardEnable;
	VkPolygonMode polygonMode;
	VkCullModeFlags cullMode;
	VkFrontFace frontFace;
	VkBool32 depthBiasEnable;
	float depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor,
		lineWidth;
};

struct multisample_state {
	VkSampleCountFlagBits rasterizationSamples;
	VkBool32 sampleShadingEnable;
	float minSampleShading;
	std::vector<VkSampleMask> sampleMask;
	VkBool32 alphaToCoverageEnable, alphaToOneEnable;
};

struct depth_stencil_state {
	VkBool32 depthTestEnable, depthWriteEnable;
	VkCompareOp depthCompareOp;
	VkBool32 depthBoundsTestEnable, stencilTestEnable;
	VkStencilOpState front, back;
	float minDepthBounds, maxDepthBounds;
};

struct color_blend_state {
	VkBool32 logicOpEnable;
	VkLogicOp logicOp;
	std::vector<VkPipelineColorBlendAttachmentState> attachments;
	float blendConstants[4];
};

struct dynamic_state {
	std::vector<VkDynamicState> dynamicStates;
};

struct pipeline_type
	: internal::movable_destructible_with_parent<VkPipeline,
		device::device_type, vkDestroyPipeline> {
	friend pipeline_type create_graphics(const type::supplier<device::device_type> &device,
		const pipeline_cache::pipeline_cache_type &pipeline_cache,
		VkPipelineCreateFlags flags,
		const std::vector<shader_stage_type> &stages,
		const vertex_input_state &vertexInputState,
		const input_assembly_state &inputAssemblyState,
		const tessellation_state *tessellationState,
		const viewport_state_type &viewportState,
		const rasterization_state &rasterizationState,
		const multisample_state &multisampleState,
		const depth_stencil_state &depthStencilState,
		const color_blend_state &colorBlendState,
		const dynamic_state &dynamicState,
		const type::supplier<pipeline_layout::pipeline_layout_type> &layout,
		const type::supplier<render_pass::render_pass_type> &render_pass,
		uint32_t subpass,
		const type::supplier<pipeline::pipeline_type> &basePipelineHandle);
	friend VCC_LIBRARY pipeline_type create_compute(
		const type::supplier<device::device_type> &device,
		const pipeline_cache::pipeline_cache_type &pipeline_cache,
		VkPipelineCreateFlags flags,
		const shader_stage_type &stage,
		const type::supplier<pipeline_layout::pipeline_layout_type> &layout,
		const type::supplier<pipeline::pipeline_type> &basePipelineHandle);

	pipeline_type() = default;
	pipeline_type(pipeline_type &&) = default;
	pipeline_type(const pipeline_type &) = default;
	pipeline_type &operator=(const pipeline_type&) = delete;
	pipeline_type &operator=(pipeline_type&&) = default;

private:
	pipeline_type(VkPipeline instance,
		const type::supplier<device::device_type> &parent,
		const type::supplier<pipeline_layout::pipeline_layout_type> &layout,
		const type::supplier<render_pass::render_pass_type> &render_pass)
		: internal::movable_destructible_with_parent<VkPipeline,
			device::device_type, vkDestroyPipeline>(instance, parent),
		layout(layout), render_pass(render_pass) {}
	pipeline_type(VkPipeline instance,
		const type::supplier<device::device_type> &parent,
		const type::supplier<pipeline_layout::pipeline_layout_type> &layout)
		: internal::movable_destructible_with_parent<VkPipeline,
		device::device_type, vkDestroyPipeline>(instance, parent),
		layout(layout) {}
	type::supplier<pipeline_layout::pipeline_layout_type> layout;
	// TODO(gardell): Remove render_pass, move into generic reference.
	type::supplier<render_pass::render_pass_type> render_pass;
};

VCC_LIBRARY pipeline_type create_graphics(const type::supplier<device::device_type> &device,
	const pipeline_cache::pipeline_cache_type &pipeline_cache,
	VkPipelineCreateFlags flags,
	const std::vector<shader_stage_type> &stages,
	const vertex_input_state &vertexInputState,
	const input_assembly_state &inputAssemblyState,
	const tessellation_state &tessellationState,
	const viewport_state_type &viewportState,
	const rasterization_state &rasterizationState,
	const multisample_state &multisampleState,
	const depth_stencil_state &depthStencilState,
	const color_blend_state &colorBlendState,
	const dynamic_state &dynamicState,
	const type::supplier<pipeline_layout::pipeline_layout_type> &layout,
	const type::supplier<render_pass::render_pass_type> &render_pass,
	uint32_t subpass,
	const type::supplier<pipeline::pipeline_type> &basePipelineHandle =
		type::supplier<pipeline::pipeline_type>());

VCC_LIBRARY pipeline_type create_graphics(const type::supplier<device::device_type> &device,
	const pipeline_cache::pipeline_cache_type &pipeline_cache,
	VkPipelineCreateFlags flags,
	const std::vector<shader_stage_type> &stages,
	const vertex_input_state &vertexInputState,
	const input_assembly_state &inputAssemblyState,
	const viewport_state_type &viewportState,
	const rasterization_state &rasterizationState,
	const multisample_state &multisampleState,
	const depth_stencil_state &depthStencilState,
	const color_blend_state &colorBlendState,
	const dynamic_state &dynamicState,
	const type::supplier<pipeline_layout::pipeline_layout_type> &layout,
	const type::supplier<render_pass::render_pass_type> &render_pass,
	uint32_t subpass,
	const type::supplier<pipeline::pipeline_type> &basePipelineHandle = type::supplier<pipeline::pipeline_type>());

VCC_LIBRARY pipeline_type create_compute(const type::supplier<device::device_type> &device,
	const pipeline_cache::pipeline_cache_type &pipeline_cache,
	VkPipelineCreateFlags flags,
	const shader_stage_type &stage,
	const type::supplier<pipeline_layout::pipeline_layout_type> &layout,
	const type::supplier<pipeline::pipeline_type> &basePipelineHandle = type::supplier<pipeline::pipeline_type>());

}  // namespace pipeline_cache
}  // namespace vcc


#endif /* PIPELINE_H_ */
