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
#include <iostream>
#include <iterator>
#include <vcc/pipeline.h>

namespace vcc {
namespace pipeline {

VkPipelineShaderStageCreateInfo convert_shader_stage(const shader_stage_type &stage) {
	VkPipelineShaderStageCreateInfo converted_stage = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0 };
	converted_stage.stage = stage.stage;
	converted_stage.module = internal::get_instance(*stage.module);
	converted_stage.pName = stage.name.c_str();
	return converted_stage;
}

VkSpecializationInfo convert_specialization_info(const shader_stage_type &stage) {
	VkSpecializationInfo specialization;
	specialization.mapEntryCount = (uint32_t) stage.map_entries.size();
	specialization.pMapEntries = stage.map_entries.data();
	specialization.dataSize = stage.data.size();
	specialization.pData = stage.data.data();
	return specialization;
}

std::pair<std::vector<VkPipelineShaderStageCreateInfo>, std::vector<VkSpecializationInfo>>
		convert_shader_stages(const std::vector<shader_stage_type> &stages) {
	std::vector<VkPipelineShaderStageCreateInfo> converted_stages;
	std::vector<VkSpecializationInfo> converted_specializations;
	converted_specializations.reserve(stages.size());
	std::transform(stages.begin(), stages.end(),
		std::back_inserter(converted_specializations),
		&convert_specialization_info);
	converted_stages.reserve(stages.size());
	std::transform(stages.begin(), stages.end(),
		std::back_inserter(converted_stages), &convert_shader_stage);
	for (std::size_t i = 0; i < converted_stages.size(); ++i) {
		converted_stages[i].pSpecializationInfo = &converted_specializations[i];
	}
	return std::make_pair(std::move(converted_stages),
		std::move(converted_specializations));
}

pipeline_type create_graphics(const type::supplier<const device::device_type> &device,
		const pipeline_cache::pipeline_cache_type &pipeline_cache, VkPipelineCreateFlags flags,
		const std::vector<shader_stage_type> &stages, const vertex_input_state &vertexInputState,
		const input_assembly_state &inputAssemblyState,
		const tessellation_state *tessellationState, const viewport_state_type &viewportState,
		const rasterization_state &rasterizationState, const multisample_state &multisampleState,
		const depth_stencil_state &depthStencilState, const color_blend_state &colorBlendState,
		const dynamic_state &dynamicState,
		const type::supplier<const pipeline_layout::pipeline_layout_type> &layout,
		const type::supplier<const render_pass::render_pass_type> &render_pass, uint32_t subpass,
		const type::supplier<const pipeline::pipeline_type> &basePipelineHandle) {
	VkPipeline pipeline;
	VkGraphicsPipelineCreateInfo create =
		{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, NULL };
	create.flags = flags;
	create.stageCount = (uint32_t) stages.size();
	std::vector<VkPipelineShaderStageCreateInfo> converted_shader_stages;
	std::vector<VkSpecializationInfo> converted_specialization_info;
	std::tie(converted_shader_stages, converted_specialization_info) =
		convert_shader_stages(stages);
	create.pStages = converted_shader_stages.data();
	VkPipelineVertexInputStateCreateInfo vertex_input_state =
			{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, NULL, 0};
	vertex_input_state.vertexBindingDescriptionCount =
			(uint32_t) vertexInputState.vertexBindingDescriptions.size();
	vertex_input_state.pVertexBindingDescriptions =
		vertexInputState.vertexBindingDescriptions.data();
	vertex_input_state.vertexAttributeDescriptionCount =
		(uint32_t) vertexInputState.vertexAttributeDescriptions.size();
	vertex_input_state.pVertexAttributeDescriptions =
		vertexInputState.vertexAttributeDescriptions.data();
	create.pVertexInputState = &vertex_input_state;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state =
		{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, NULL, 0};
	input_assembly_state.topology = inputAssemblyState.topology;
	input_assembly_state.primitiveRestartEnable = inputAssemblyState.primitiveRestartEnable;
	create.pInputAssemblyState = &input_assembly_state;

	VkPipelineTessellationStateCreateInfo tessellation_state =
		{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, NULL, 0};
	if (tessellationState) {
		tessellation_state.patchControlPoints = tessellationState->patchControlPoints;
		create.pTessellationState = &tessellation_state;
	} else {
		create.pTessellationState = NULL;
	}

    VkPipelineViewportStateCreateInfo viewport_state =
		{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, NULL, 0};
	viewport_state.viewportCount = (uint32_t) viewportState.viewports.size();
	viewport_state.pViewports = viewportState.viewports.data();
	viewport_state.scissorCount = (uint32_t) viewportState.scissors.size();
	viewport_state.pScissors = viewportState.scissors.data();
	create.pViewportState = &viewport_state;

	VkPipelineRasterizationStateCreateInfo rasterization_state =
		{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, NULL, 0};
	rasterization_state.depthClampEnable = rasterizationState.depthClampEnable;
	rasterization_state.rasterizerDiscardEnable = rasterizationState.rasterizerDiscardEnable;
	rasterization_state.polygonMode = rasterizationState.polygonMode;
	rasterization_state.cullMode = rasterizationState.cullMode;
	rasterization_state.frontFace = rasterizationState.frontFace;
	rasterization_state.depthBiasEnable = rasterizationState.depthBiasEnable;
	rasterization_state.depthBiasConstantFactor = rasterizationState.depthBiasConstantFactor;
	rasterization_state.depthBiasClamp = rasterizationState.depthBiasClamp;
	rasterization_state.depthBiasSlopeFactor = rasterizationState.depthBiasSlopeFactor;
	rasterization_state.lineWidth = rasterizationState.lineWidth;
	create.pRasterizationState = &rasterization_state;

	VkPipelineMultisampleStateCreateInfo multisample_state =
		{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, NULL, 0};
	multisample_state.rasterizationSamples = multisampleState.rasterizationSamples;
	multisample_state.sampleShadingEnable = multisampleState.sampleShadingEnable;
	multisample_state.minSampleShading = multisampleState.minSampleShading;
	multisample_state.pSampleMask = multisampleState.sampleMask.data();
	multisample_state.alphaToCoverageEnable = multisampleState.alphaToCoverageEnable;
	multisample_state.alphaToOneEnable = multisampleState.alphaToOneEnable;
	create.pMultisampleState = &multisample_state;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state =
		{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, NULL, 0};
	depth_stencil_state.depthTestEnable = depthStencilState.depthTestEnable;
	depth_stencil_state.depthWriteEnable = depthStencilState.depthWriteEnable;
	depth_stencil_state.depthCompareOp = depthStencilState.depthCompareOp;
	depth_stencil_state.depthBoundsTestEnable = depthStencilState.depthBoundsTestEnable;
	depth_stencil_state.stencilTestEnable = depthStencilState.stencilTestEnable;
	depth_stencil_state.front = depthStencilState.front;
	depth_stencil_state.back = depthStencilState.back;
	depth_stencil_state.minDepthBounds = depthStencilState.minDepthBounds;
	depth_stencil_state.maxDepthBounds = depthStencilState.maxDepthBounds;
	create.pDepthStencilState = &depth_stencil_state;

	VkPipelineColorBlendStateCreateInfo color_blend_state =
		{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, NULL, 0};
	color_blend_state.logicOpEnable = colorBlendState.logicOpEnable;
	color_blend_state.logicOp = colorBlendState.logicOp;
	color_blend_state.attachmentCount = (uint32_t) colorBlendState.attachments.size();
	color_blend_state.pAttachments = colorBlendState.attachments.data();
	std::copy(&colorBlendState.blendConstants[0],
		&colorBlendState.blendConstants[0]
		+ sizeof(colorBlendState.blendConstants)
		/ sizeof(colorBlendState.blendConstants[0]),
		&color_blend_state.blendConstants[0]);
	create.pColorBlendState = &color_blend_state;

	VkPipelineDynamicStateCreateInfo dynamic_state =
		{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, NULL, 0};
	dynamic_state.dynamicStateCount = (uint32_t) dynamicState.dynamicStates.size();
	dynamic_state.pDynamicStates = dynamicState.dynamicStates.data();
	create.pDynamicState = &dynamic_state;

	create.layout = internal::get_instance(*layout);
	create.renderPass = internal::get_instance(*render_pass);
	create.subpass = subpass;
	if (basePipelineHandle) {
		create.basePipelineHandle = internal::get_instance(*basePipelineHandle);
	} else {
		create.basePipelineHandle = VK_NULL_HANDLE;
	}
	create.basePipelineIndex = -1;

	VKCHECK(vkCreateGraphicsPipelines(internal::get_instance(*device),
		internal::get_instance(pipeline_cache), 1, &create, NULL, &pipeline));
	return pipeline_type(pipeline, device, layout, render_pass);
}

pipeline_type create_graphics(const type::supplier<const device::device_type> &device,
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
		const type::supplier<const pipeline_layout::pipeline_layout_type> &layout,
		const type::supplier<const render_pass::render_pass_type> &render_pass,
		uint32_t subpass,
		const type::supplier<const pipeline::pipeline_type> &basePipelineHandle) {
	return create_graphics(device, pipeline_cache, flags, stages,
		vertexInputState, inputAssemblyState, &tessellationState,
		viewportState, rasterizationState, multisampleState, depthStencilState,
		colorBlendState, dynamicState, layout, render_pass, subpass,
		basePipelineHandle);
}

pipeline_type create_graphics(const type::supplier<const device::device_type> &device,
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
	const type::supplier<const pipeline_layout::pipeline_layout_type> &layout,
	const type::supplier<const render_pass::render_pass_type> &render_pass,
	uint32_t subpass,
	const type::supplier<const pipeline::pipeline_type> &basePipelineHandle) {
	return create_graphics(device, pipeline_cache, flags, stages,
		vertexInputState, inputAssemblyState, nullptr, viewportState,
		rasterizationState, multisampleState, depthStencilState,
		colorBlendState, dynamicState, layout, render_pass, subpass,
		basePipelineHandle);
}

pipeline_type create_compute(const type::supplier<const device::device_type> &device,
		const pipeline_cache::pipeline_cache_type &pipeline_cache,
		VkPipelineCreateFlags flags,
		const shader_stage_type &stage,
		const type::supplier<const pipeline_layout::pipeline_layout_type> &layout,
		const type::supplier<const pipeline::pipeline_type> &basePipelineHandle) {
	VkComputePipelineCreateInfo create = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, NULL, };
	create.flags = flags;
	create.stage = convert_shader_stage(stage);
	create.layout = internal::get_instance(*layout);
	if (basePipelineHandle) {
		create.basePipelineHandle = internal::get_instance(*basePipelineHandle);
	} else {
		create.basePipelineHandle = VK_NULL_HANDLE;
	}
	create.basePipelineIndex = -1;
	VkPipeline instance;
	VKCHECK(vkCreateComputePipelines(internal::get_instance(*device),
		internal::get_instance(pipeline_cache), 1, &create, NULL, &instance));
	return pipeline_type(instance, device, layout);
}

}  // namespace pipeline
}  // namespace vcc
