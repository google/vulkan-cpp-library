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
#include <vcc/data/buffer.h>

#include <cassert>
#include <chrono>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <type/types.h>
#include <vcc/buffer.h>
#include <vcc/command_buffer.h>
#include <vcc/command_pool.h>
#include <vcc/descriptor_pool.h>
#include <vcc/descriptor_set.h>
#include <vcc/descriptor_set_update.h>
#include <vcc/descriptor_set_layout.h>
#include <vcc/device.h>
#include <vcc/enumerate.h>
#include <vcc/framebuffer.h>
#include <vcc/instance.h>
#include <vcc/memory.h>
#include <vcc/physical_device.h>
#include <vcc/pipeline.h>
#include <vcc/pipeline_cache.h>
#include <vcc/pipeline_layout.h>
#include <vcc/queue.h>
#include <vcc/render_pass.h>
#include <vcc/semaphore.h>
#include <vcc/shader_module.h>
#include <vcc/surface.h>
#include <vcc/swapchain.h>
#include <vcc/window.h>

namespace teapot {

#include "teapot.h"

}  // namespace teapot

const bool validate = true;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow) {

	vcc::instance::instance_type instance;
	{
		const std::set<std::string> extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		};
		assert(vcc::enumerate::contains_all(
			vcc::enumerate::instance_extension_properties(""),
			extensions));
		instance = vcc::instance::create({}, extensions);
	}

	vcc::device::device_type device;
	{
		const VkPhysicalDevice physical_device(
			vcc::physical_device::enumerate(instance).front());

		const std::set<std::string> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		assert(vcc::enumerate::contains_all(
			vcc::enumerate::device_extension_properties(physical_device, ""),
			extensions));

		device = vcc::device::create(physical_device,
		{ vcc::device::queue_create_info_type{
			vcc::physical_device::get_queue_family_properties_with_flag(
			vcc::physical_device::queue_famility_properties(physical_device),
				VK_QUEUE_GRAPHICS_BIT),
				{ 0 } }
		},
		{}, extensions, {});
	}

	type::mat4 projection_matrix;
	type::mat4 modelview_matrix;
	type::mat3 normal_matrix;

	vcc::data::buffer_type matrix_uniform_buffer(vcc::data::create(
		type::linear, std::ref(device), 0, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE, {},
		std::ref(projection_matrix), std::ref(modelview_matrix),
		std::ref(normal_matrix)));
	vcc::memory::bind(std::ref(device), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		matrix_uniform_buffer);

	struct {
		type::vec4 position;
		type::vec3 attenuation;
		type::vec3 spot_direction;
		type::float_type spot_cos_cutoff;
		type::vec4 ambient;
		type::vec4 diffuse;
		type::vec4 specular;
		type::float_type spot_exponent;
	} light{
		type::vec4(glm::vec4(0.f, 10.f, 0.f, 1.f)),
		type::vec3(glm::vec3(1, 0, 0)), type::vec3(glm::vec3(0, 0, -1)),
		type::float_type(-1), type::vec4(glm::vec4(0.2f, 0.2f, 0.2f, 1)),
		type::vec4(glm::vec4(0.9f, 0.9f, 0.9f, 1.f)),
		type::vec4(glm::vec4(1.f, 1.f, 1.f, 1.f)), type::float_type(120.f)
	};

	vcc::data::buffer_type light_uniform_buffer(vcc::data::create(
		type::linear_std140, std::ref(device), 0,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, {},
		std::ref(light.position), std::ref(light.attenuation),
		std::ref(light.spot_direction), std::ref(light.spot_cos_cutoff),
		std::ref(light.ambient), std::ref(light.diffuse),
		std::ref(light.specular), std::ref(light.spot_exponent)));
	vcc::memory::bind(std::ref(device), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		light_uniform_buffer);

	vcc::data::buffer_type vertex_buffer(vcc::data::create(
		type::interleaved_std140, std::ref(device), 0,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, {},
		std::ref(teapot::vertices), std::ref(teapot::normals)));
	vcc::memory::bind(std::ref(device), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		vertex_buffer);
	vcc::data::buffer_type index_buffer(vcc::data::create(
		type::linear, std::ref(device), 0, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE, {}, std::ref(teapot::indices)));
	vcc::memory::bind(std::ref(device), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		index_buffer);

	vcc::descriptor_set_layout::descriptor_set_layout_type desc_layout(
		vcc::descriptor_set_layout::create(std::ref(device),
		{
			vcc::descriptor_set_layout::descriptor_set_layout_binding{ 0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
			VK_SHADER_STAGE_VERTEX_BIT,{} },
			vcc::descriptor_set_layout::descriptor_set_layout_binding{ 1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
			VK_SHADER_STAGE_FRAGMENT_BIT,{} }
		}));
	vcc::descriptor_pool::descriptor_pool_type desc_pool(
		vcc::descriptor_pool::create(std::ref(device), 0, 1,
		{ VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 } }));
	vcc::descriptor_set::descriptor_set_type desc_set(std::move(
		vcc::descriptor_set::create(std::ref(device), std::ref(desc_pool),
		{ std::ref(desc_layout) }).front()));

	vcc::descriptor_set::update(device,
		vcc::descriptor_set::write_buffer(std::ref(desc_set), 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		{ vcc::descriptor_set::buffer_info(std::ref(matrix_uniform_buffer)) }),
		vcc::descriptor_set::write_buffer(std::ref(desc_set), 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		{ vcc::descriptor_set::buffer_info(std::ref(light_uniform_buffer)) }));

	vcc::queue::queue_type queue(vcc::queue::get_graphics_queue(
		std::ref(device)));
	vcc::window::window_type window(vcc::window::create(
		hInstance, std::ref(instance), std::ref(device), std::ref(queue),
		VkExtent2D{ 500, 500 }, VK_FORMAT_A8B8G8R8_UINT_PACK32,
		"Lighting demo"));

	const glm::mat4 view_matrix(glm::lookAt(glm::vec3(0.0f, 6.0f, 6.0f),
		glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0)));
	float x_angle(0), y_angle(0);
	bool mouse_down(false);
	int last_x, last_y;
	const float scale_x(128), scale_y(128);

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	vcc::render_pass::render_pass_type render_pass(vcc::render_pass::create(
		std::ref(device),
		{
			VkAttachmentDescription{ 0, vcc::window::get_format(window),
			VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			VkAttachmentDescription{ 0, depth_format, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		},
		{
			vcc::render_pass::subpass_description_type{
				{},
				{ VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } },
				{},
				VkAttachmentReference{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
				{}
			}
		},
		{}));

	vcc::pipeline_cache::pipeline_cache_type pipeline_cache(
		vcc::pipeline_cache::create(std::ref(device)));
	vcc::shader_module::shader_module_type vert_shader_module(vcc::shader_module::create(
		std::ref(device),
		std::ifstream("../../../lighting-vert.spv", std::ios_base::binary | std::ios_base::in)));
	vcc::shader_module::shader_module_type frag_shader_module(vcc::shader_module::create(
		std::ref(device),
		std::ifstream("../../../lighting-frag.spv", std::ios_base::binary | std::ios_base::in)));
	vcc::pipeline_layout::pipeline_layout_type pipeline_layout(
		vcc::pipeline_layout::create(std::ref(device), { std::ref(desc_layout) }));
	vcc::pipeline::pipeline_type pipeline(vcc::pipeline::create_graphics(
		std::ref(device), pipeline_cache, 0,
		{
			vcc::pipeline::shader_stage(VK_SHADER_STAGE_VERTEX_BIT,
				std::ref(vert_shader_module), "main"),
			vcc::pipeline::shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT,
				std::ref(frag_shader_module), "main")
		},
		vcc::pipeline::vertex_input_state{
			{
				VkVertexInputBindingDescription{ 0, sizeof(glm::vec4) * 2,
					VK_VERTEX_INPUT_RATE_VERTEX }
			},
			{
				VkVertexInputAttributeDescription{ 0, 0,
					VK_FORMAT_R32G32B32_SFLOAT, 0 },
				VkVertexInputAttributeDescription{ 1, 0,
					VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec4) },
			}
		},
		vcc::pipeline::input_assembly_state{
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE },
		vcc::pipeline::viewport_state(1, 1),
		vcc::pipeline::rasterization_state{ VK_FALSE, VK_FALSE,
			VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0, 0, 0, 0 },
		vcc::pipeline::multisample_state{ VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0,
			{}, VK_FALSE, VK_FALSE },
		vcc::pipeline::depth_stencil_state{
			VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE, VK_FALSE,
			{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
				VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
				VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
			0, 0 },
		vcc::pipeline::color_blend_state{ VK_FALSE, VK_LOGIC_OP_CLEAR,
			{ VkPipelineColorBlendAttachmentState{ VK_FALSE,
				VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
				0xF } },
			{ 0, 0, 0, 0 } },
		vcc::pipeline::dynamic_state{ { VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR } },
		std::ref(pipeline_layout), std::ref(render_pass), 0));

	vcc::command_pool::command_pool_type cmd_pool(vcc::command_pool::create(
		std::ref(device), 0, vcc::queue::get_family_index(queue)));
	std::vector<vcc::command_buffer::command_buffer_type> command_buffers;
	return vcc::window::run(window,
		[&](VkExtent2D extent, VkFormat format, std::vector<vcc::window::swapchain_type> &swapchain_images) {
			type::mutate(projection_matrix)[0] =
				glm::perspective(45.f, float(extent.width) / extent.height,
					1.f, 100.f);
			command_buffers.clear();

			std::shared_ptr<vcc::image::image_type> depth_image(
				std::make_shared<vcc::image::image_type>(vcc::image::create(
					std::ref(device), 0, VK_IMAGE_TYPE_2D, depth_format,
					{ extent.width, extent.height, 1 }, 1, 1,
					VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					VK_SHARING_MODE_EXCLUSIVE, {}, VK_IMAGE_LAYOUT_UNDEFINED)));
			vcc::memory::bind(std::ref(device),
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *depth_image);

			vcc::command_buffer::command_buffer_type command_buffer(std::move(
				vcc::command_buffer::allocate(std::ref(device), std::ref(cmd_pool),
					VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));
			vcc::command_buffer::compile(command_buffer, 0, VK_FALSE, 0, 0,
				vcc::command_buffer::pipeline_barrier(
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, {}, {},
					{ vcc::command_buffer::image_memory_barrier{ 0,
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
						depth_image,
						{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 } } }));
			vcc::queue::submit(queue, {}, { std::ref(command_buffer) }, {});
			vcc::queue::wait_idle(queue);

			command_buffers = vcc::command_buffer::allocate(std::ref(device),
				std::ref(cmd_pool), VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				(uint32_t) swapchain_images.size());
			for (std::size_t i = 0; i < swapchain_images.size(); ++i) {
				vcc::command_buffer::compile(command_buffers[i], 0, VK_FALSE, 0, 0,
					vcc::command_buffer::render_pass(
						std::ref(render_pass),
						vcc::framebuffer::create(std::ref(device),
							std::ref(render_pass),
							{
								std::ref(vcc::window::get_image_view(
									swapchain_images[i])),
								vcc::image_view::create(depth_image,
									{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 })
							}, extent, 1),
						VkRect2D{ 0, 0, extent.width, extent.height },
						{
							vcc::command_buffer::clear_color(
								VkClearColorValue{ { .2f, .2f, .2f, .2f } }),
							vcc::command_buffer::clear_depth_stencil(
								VkClearDepthStencilValue{ 1, 0 })
						},
						VK_SUBPASS_CONTENTS_INLINE,
						vcc::command_buffer::bind_pipeline{
							VK_PIPELINE_BIND_POINT_GRAPHICS, std::ref(pipeline) },
						vcc::command_buffer::bind_vertex_data_buffers(
							{ std::ref(vertex_buffer) }, { 0, 0 }),
						vcc::command_buffer::bind_index_data_buffer(
							std::ref(index_buffer), 0, VK_INDEX_TYPE_UINT16),
						vcc::command_buffer::bind_descriptor_sets{
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							std::ref(pipeline_layout), 0,
							{ std::ref(desc_set) }, {} },
						vcc::command_buffer::set_viewport{ 0, {
							VkViewport{ 0.f, 0.f, float(extent.width),
								float(extent.height), 0.f, 1.f } } },
						vcc::command_buffer::set_scissor{ 0,{ VkRect2D{
							VkOffset2D{ 0, 0 }, extent } } },
						vcc::command_buffer::draw_indexed{
							(uint32_t)teapot::indices.size(), 1, 0, 0, 0 }));
			}
		},
		[&](uint32_t index) {
			{
				auto mutated_modelview_matrix(type::mutate(modelview_matrix));
				mutated_modelview_matrix[0] = view_matrix
					* glm::rotate(y_angle, glm::vec3(1, 0, 0))
					* glm::rotate(x_angle, glm::vec3(0, 1, 0));
				auto mutated_normal_matrix(type::mutate(normal_matrix));
				mutated_normal_matrix[0] = glm::mat3(glm::transpose(
					glm::inverse(mutated_modelview_matrix[0])));
			}
			vcc::queue::submit(queue, {},
				{ std::ref(command_buffers[index]) }, {});
		},
		vcc::window::input_callbacks_type()
		.set_mouse_down_callback([&mouse_down, &last_x, &last_y]
				(vcc::window::mouse_button_type, int x, int y) {
			last_x = x;
			last_y = y;
			mouse_down = true;
			return true;
		}).set_mouse_up_callback([&mouse_down]
				(vcc::window::mouse_button_type, int x, int y) {
			mouse_down = false;
			return true;
		}).set_mouse_move_callback([&](int x, int y) {
			if (mouse_down) {
				x_angle += (x - last_x) / scale_x;
				y_angle += (y - last_y) / scale_y;
				last_x = x;
				last_y = y;
			}
			return true;
		}));
}
