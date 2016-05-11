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
#define NOMINMAX
#include <fstream>
#include <gtest/gtest.h>
#include <type/types.h>
#include <vcc/command_pool.h>
#include <vcc/data/buffer.h>
#include <vcc/descriptor_pool.h>
#include <vcc/descriptor_set.h>
#include <vcc/descriptor_set_update.h>
#include <vcc/device.h>
#include <vcc/enumerate.h>
#include <vcc/instance.h>
#include <vcc/memory.h>
#include <vcc/physical_device.h>
#include <vcc/pipeline_layout.h>
#include <vcc/queue.h>
#include <vcc/shader_module.h>

TEST(ComputeShaderIntegrationTest, ComputeShaderIntegrationTest1) {
	vcc::instance::instance_type instance(vcc::instance::create({}, {}));
	const VkPhysicalDevice physical_device(
		vcc::physical_device::enumerate(instance).front());
	vcc::device::device_type device(vcc::device::create(physical_device,
		{ vcc::device::queue_create_info_type{
			vcc::physical_device::get_queue_family_properties_with_flag(
				vcc::physical_device::queue_famility_properties(
					physical_device),
				VK_QUEUE_COMPUTE_BIT),
				{ 0 } }
		}, {}, {}, {}));

	vcc::descriptor_set_layout::descriptor_set_layout_type desc_layout(
		vcc::descriptor_set_layout::create(std::ref(device),
		{
			vcc::descriptor_set_layout::descriptor_set_layout_binding{ 0,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
				VK_SHADER_STAGE_COMPUTE_BIT,{} },
			vcc::descriptor_set_layout::descriptor_set_layout_binding{ 1,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
				VK_SHADER_STAGE_COMPUTE_BIT,{} }
		}));

	vcc::pipeline_layout::pipeline_layout_type pipeline_layout(vcc::pipeline_layout::create(
		std::ref(device), { std::ref(desc_layout) }));

	vcc::descriptor_pool::descriptor_pool_type desc_pool(
		vcc::descriptor_pool::create(std::ref(device), 0, 1,
			{ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 } }));

	vcc::descriptor_set::descriptor_set_type desc_set(std::move(
		vcc::descriptor_set::create(std::ref(device),
			std::ref(desc_pool),
			{ std::ref(desc_layout) }).front()));

	const std::size_t num_elements(16);
	type::float_array input_array(num_elements);
	{
		const auto mutate(type::mutate(input_array));
		std::iota(std::begin(mutate), std::end(mutate), 1.f);
	}
	vcc::data::buffer_type input_buffer(vcc::data::create(
		type::linear_std140, std::ref(device), 0, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE, {}, std::ref(input_array)));
	const type::supplier<vcc::memory::memory_type> input_memory(
		vcc::memory::bind(std::ref(device), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			input_buffer));

	vcc::buffer::buffer_type output_buffer(
		vcc::buffer::create(std::ref(device), 0,
			num_elements * sizeof(decltype(input_array)::value_type),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, {}));
	const type::supplier<vcc::memory::memory_type> output_memory(
		vcc::memory::bind(std::ref(device), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			output_buffer));

	vcc::descriptor_set::update(device,
		vcc::descriptor_set::write_buffer(std::ref(desc_set), 0, 0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			{ vcc::descriptor_set::buffer_info(std::ref(input_buffer)) }));
	vcc::descriptor_set::update(device,
		vcc::descriptor_set::write_buffer(std::ref(desc_set), 1, 0,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			{ vcc::descriptor_set::buffer_info(std::ref(output_buffer), 0,
				VK_WHOLE_SIZE) }));

	vcc::queue::queue_type queue(vcc::queue::get_queue(
		std::ref(device), VK_QUEUE_COMPUTE_BIT));

	vcc::shader_module::shader_module_type shader_module(
		vcc::shader_module::create(std::ref(device),
			std::ifstream("../../compute-shader-integration-test-1.spv",
				std::ios_base::binary | std::ios_base::in)));

	vcc::pipeline_cache::pipeline_cache_type pipeline_cache(
		vcc::pipeline_cache::create(std::ref(device)));
	vcc::pipeline::pipeline_type pipeline(
		vcc::pipeline::create_compute(std::ref(device),
			pipeline_cache, 0, vcc::pipeline::shader_stage(VK_SHADER_STAGE_COMPUTE_BIT,
				std::ref(shader_module), "main"),
			std::ref(pipeline_layout)));

	vcc::command_pool::command_pool_type cmd_pool(vcc::command_pool::create(
		std::ref(device), 0, vcc::queue::get_family_index(queue)));

	vcc::command_buffer::command_buffer_type command_buffer(std::move(
		vcc::command_buffer::allocate(std::ref(device),
			std::ref(cmd_pool), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));

	vcc::command_buffer::compile(command_buffer, 0, VK_FALSE,
		0, 0,
		vcc::command_buffer::bind_pipeline{
			VK_PIPELINE_BIND_POINT_COMPUTE, std::ref(pipeline) },
		vcc::command_buffer::bind_descriptor_sets{
			VK_PIPELINE_BIND_POINT_COMPUTE,
			std::ref(pipeline_layout), 0,
			{ std::ref(desc_set) },{} },
		vcc::command_buffer::dispatch{1, 1, 1},
		vcc::command_buffer::pipeline_barrier(
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_HOST_BIT, 0, {}, {
				vcc::command_buffer::buffer_memory_barrier(
					VK_ACCESS_UNIFORM_READ_BIT, VK_ACCESS_HOST_READ_BIT,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					std::ref(output_buffer))
			},
			{}));
	vcc::fence::fence_type fence(vcc::fence::create(std::ref(device)));
	vcc::queue::submit(queue, {},
		{ std::ref(command_buffer) }, {}, fence);
	vcc::fence::wait(std::ref(device), { std::ref(fence) }, true,
		std::chrono::nanoseconds::max());
	vcc::queue::wait_idle(queue);

	{
		vcc::memory::map_type map(vcc::memory::map(std::ref(output_memory)));
		const auto output_ptr(
			reinterpret_cast<decltype(input_array)::value_type *>(map.data));
		for (int i = 0; i < num_elements; ++i) {
			ASSERT_FLOAT_EQ(2 * input_array[i], output_ptr[i]);
		}
	}
}
