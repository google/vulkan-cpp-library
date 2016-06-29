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
#include <cassert>
#include <sstream>
#include <vcc/command.h>
#include <vcc/internal/loader.h>
#include <vcc/memory.h>
#include <vcc/queue.h>
#include <vcc/util.h>
#include <vr.h>

namespace {

glm::mat4x3 convert(const vr::HmdMatrix34_t &m) {
	return glm::mat4x3(
		m.m[0][0], m.m[1][0], m.m[2][0],
		m.m[0][1], m.m[1][1], m.m[2][1],
		m.m[0][2], m.m[1][2], m.m[2][2],
		m.m[0][3], m.m[1][3], m.m[2][3]);
}

glm::mat4 convert(const vr::HmdMatrix44_t &m) {
	return glm::mat4(
		m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
		m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
		m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
		m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
}

}  // anonymous namespace

vr::IVRSystem *vr_type::init_hmd() {
	vr::EVRInitError eError(vr::VRInitError_None);
	vr::IVRSystem *hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None) {
		VCC_PRINT("Failed to initialize the VR system: %s",
			vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		throw std::runtime_error("VR_Init failed");
	}
	return hmd;
}

vr_type::vr_type(const char *window_title, uint32_t window_width,
		uint32_t window_height,
		const type::supplier<vcc::queue::queue_type> &queue)
	: hmd(init_hmd()), queue(queue),
	  glut(window_title, window_width, window_height),
	  window_width(window_width), window_height(window_height),
	  extent(get_recommended_render_target_size()) {

	if (!GLEW_NV_draw_vulkan_image) {
		throw std::runtime_error("Missing GL_NV_draw_vulkan_image is required");
	}

	VCC_PRINT("driver: %s, display: %s",
		get_string_tracked_device_property(vr::k_unTrackedDeviceIndex_Hmd,
			vr::Prop_TrackingSystemName_String).c_str(),
		get_string_tracked_device_property(vr::k_unTrackedDeviceIndex_Hmd,
			vr::Prop_SerialNumber_String).c_str());

	if (!vr::VRCompositor()) {
		VCC_PRINT("Compositor initialization failed. See log file for details");
		throw std::runtime_error("VRCompositor failed");
	}

	type::supplier<vcc::device::device_type> device(
		vcc::internal::get_parent(*queue));
	image = vcc::image::image_type(vcc::image::create(device, 0,
		VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
		{ extent.width, extent.height, 1 }, 1, 1,
		VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE, {}, VK_IMAGE_LAYOUT_UNDEFINED));
	vcc::memory::bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		std::ref(image));
	image_view = vcc::image_view::create(std::ref(image),
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
}

vr_type::~vr_type() {
	vr::VR_Shutdown();
}

int vr_type::run(const draw_callback_type &draw_callback,
		const event_callback_type &event_callback) {
	vcc::command_pool::command_pool_type command_pool(
		vcc::command_pool::create(vcc::internal::get_parent(*queue),
			0, vcc::queue::get_family_index(*queue)));
	vcc::semaphore::semaphore_type pre_draw_semaphore(vcc::semaphore::create(
		vcc::internal::get_parent(*queue))), post_draw_semaphore(
			vcc::semaphore::create(vcc::internal::get_parent(*queue)));
	vcc::command_buffer::command_buffer_type pre_draw_command_buffer(std::move(
		vcc::command_buffer::allocate(vcc::internal::get_parent(*queue),
			std::ref(command_pool), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1)
		.front())),
		post_draw_command_buffer(std::move(vcc::command_buffer::allocate(
			vcc::internal::get_parent(*queue), std::ref(command_pool),
			VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));

	const gl::render_texture texture(extent.width, extent.height);
	const gl::framebuffer framebuffer(texture);

	vcc::command_buffer::compile(std::ref(pre_draw_command_buffer),
		0, VK_FALSE, 0, 0, vcc::command::pipeline_barrier(
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, {},
			{}, {
				vcc::command::image_memory_barrier{ 0, 0,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_QUEUE_FAMILY_IGNORED,
					vcc::queue::get_family_index(*queue),
					std::ref(image),
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				}
			}));
	vcc::command_buffer::compile(std::ref(post_draw_command_buffer),
		0, VK_FALSE, 0, 0, vcc::command::pipeline_barrier(
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, {}, {},
			{
				vcc::command::image_memory_barrier{ 0, 0,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					std::ref(image),
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				}
			}));

	std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount>
		trackedDevicePose;
	std::array<glm::mat4x3, vr::k_unMaxTrackedDeviceCount> mat4DevicePose;
	std::array<bool, vr::k_unMaxTrackedDeviceCount> bPoseIsValid;
	return glut.run(
		[&]() {
			vr::IVRCompositor *compositor(vr::VRCompositor());
			compositor->WaitGetPoses(trackedDevicePose.data(),
				vr::k_unMaxTrackedDeviceCount, NULL, 0);

			for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice) {
				bPoseIsValid[nDevice] = trackedDevicePose[nDevice].bPoseIsValid;
				if (bPoseIsValid[nDevice]) {
					mat4DevicePose[nDevice] = convert(
						trackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
				}
			}

			type::supplier<vcc::device::device_type> device(
				vcc::internal::get_parent(*queue));

			const VkSemaphore pre_draw_vk_semaphore(
				vcc::internal::get_instance(pre_draw_semaphore));
			GLCHECK(glSignalVkSemaphoreNV(
				(GLuint64)pre_draw_vk_semaphore));

			vcc::queue::submit(*queue, { { std::ref(pre_draw_semaphore),
				VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT } },
				{ std::ref(pre_draw_command_buffer) }, {});

			draw_callback(mat4DevicePose);

			vcc::queue::submit(*queue, {},
				{ std::ref(post_draw_command_buffer) },
				{ std::ref(post_draw_semaphore) });

			const VkSemaphore post_draw_vk_semaphore(
				vcc::internal::get_instance(post_draw_semaphore));
			GLCHECK(glWaitVkSemaphoreNV(
				(GLuint64)post_draw_vk_semaphore));

			framebuffer.bind(GL_DRAW_FRAMEBUFFER);
			framebuffer.complete(GL_DRAW_FRAMEBUFFER);
			GLCHECK(glViewport(0, 0, extent.width, extent.height));
			VkImage vk_image(vcc::internal::get_instance(image));
			GLCHECK(glDrawVkImageNV((GLuint64)vk_image, 0,
				0.f, 0.f,
				GLfloat(extent.width), GLfloat(extent.height), 0.f,
				0.f, 0.f, 1.f, 1.f));

			const vr::Texture_t texdesc = { (void*)texture.get_instance(),
				vr::API_OpenGL, vr::ColorSpace_Gamma };
			{
				const vr::VRTextureBounds_t bounds = { 0.f, 0.f, .5f, 1.f };
				compositor->Submit(vr::Eye_Left, &texdesc, &bounds);
			}
			{
				const vr::VRTextureBounds_t bounds = { .5f, 0.f, 1.f, 1.f };
				compositor->Submit(vr::Eye_Right, &texdesc, &bounds);
			}

			gl::framebuffer::unbind(GL_DRAW_FRAMEBUFFER);
			framebuffer.bind(GL_READ_FRAMEBUFFER);
			framebuffer.complete(GL_READ_FRAMEBUFFER);
			GLCHECK(glViewport(0, 0, window_width, window_height));
			GLCHECK(glBlitFramebuffer(0, 0, extent.width / 2, extent.height, 0, 0,
				window_width, window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR));
			glFlush();
		},
		[&]() {
			vr::VREvent_t event;
			while (hmd->PollNextEvent(&event, sizeof(event))) {
				event_callback(event);
			}

			for (vr::TrackedDeviceIndex_t unDevice = 0;
			unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
				vr::VRControllerState_t state;
				if (hmd->GetControllerState(unDevice, &state)) {
					//m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
				}
			}
		},
		[&](int width, int height) {
			window_width = width;
			window_height = height;
		});
}

VkExtent2D vr_type::get_recommended_render_target_size() const {
	VkExtent2D extent;
	hmd->GetRecommendedRenderTargetSize(&extent.width, &extent.height);
	return VkExtent2D{ 2 * extent.width, extent.height };
}

glm::mat4 vr_type::get_projection_matrix(vr::Hmd_Eye nEye, float near_z,
		float far_z) const {
	const vr::HmdMatrix44_t mat(hmd->GetProjectionMatrix(nEye, near_z, far_z,
		vr::API_OpenGL));
	return convert(mat);
}

glm::mat4 vr_type::get_head_to_eye_transform(vr::Hmd_Eye nEye) const {
	return glm::inverse(glm::mat4(convert(hmd->GetEyeToHeadTransform(nEye))));
}

std::string vr_type::get_string_tracked_device_property(
		vr::TrackedDeviceIndex_t unDeviceIndex,
		vr::ETrackedDeviceProperty prop) const {
	vr::ETrackedPropertyError error;
	const uint32_t size(hmd->GetStringTrackedDeviceProperty(unDeviceIndex,
		prop, NULL, 0, &error));
	std::string string(size, '\0');
	hmd->GetStringTrackedDeviceProperty(unDeviceIndex, prop, &string[0], size,
		&error);
	assert(error == vr::TrackedProp_Success);
	return std::move(string);
}

vr_model vr_type::load_model(const char *model_name) const {
	vr::EVRInitError eError(vr::VRInitError_None);
	vr::IVRRenderModels *render_models((vr::IVRRenderModels *)
		vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError));
	if (!render_models) {
		std::stringstream ss;
		ss << "Unable to get render model interface: "
			<< vr::VR_GetVRInitErrorAsEnglishDescription(eError) << std::endl;
		VCC_PRINT("%s", ss.str().c_str());
		throw std::runtime_error(ss.str());
	}

	vr::RenderModel_t *pModel;
	vr::EVRRenderModelError error;
	do {
		error = render_models->LoadRenderModel_Async(model_name, &pModel);
		// TODO(gardell): Is there no better way than to poll?
		Sleep(1000);
	} while (error == vr::VRRenderModelError_Loading);
	if (error != vr::VRRenderModelError_None) {
		std::stringstream ss;
		ss << "Failed to load model: " << model_name << ", error: "
			<< render_models->GetRenderModelErrorNameFromEnum(error)
			<< std::endl;
		VCC_PRINT("%s", ss.str().c_str());
		throw std::runtime_error(ss.str());
	}

	vr::RenderModel_TextureMap_t *pTexture;
	do {
		error = render_models->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
		// TODO(gardell): Is there no better way than to poll?
		Sleep(1000);
	} while (error == vr::VRRenderModelError_Loading);
	if (error != vr::VRRenderModelError_None) {
		std::stringstream ss;
		ss << "Failed to load texture for model: " << model_name << ", error: "
			<< render_models->GetRenderModelErrorNameFromEnum(error)
			<< std::endl;
		VCC_PRINT("%s", ss.str().c_str());
		throw std::runtime_error(ss.str());
	}

	type::ushort_array indices(pModel->unTriangleCount * 3);
	auto write_indices(type::write(indices));
	std::copy(pModel->rIndexData,
		pModel->rIndexData + pModel->unTriangleCount * 3,
		write_indices.begin());

	type::vec3_array vertices(pModel->unVertexCount);
	auto write_vertices(type::write(vertices));
	std::transform(pModel->rVertexData,
		pModel->rVertexData + pModel->unVertexCount,
		write_vertices.begin(),
		[](const vr::RenderModel_Vertex_t &vertex) {
		return glm::vec3(vertex.vPosition.v[0],
			vertex.vPosition.v[1], vertex.vPosition.v[2]);
	});

	type::vec3_array normals(pModel->unVertexCount);
	auto write_normals(type::write(normals));
	std::transform(pModel->rVertexData,
		pModel->rVertexData + pModel->unVertexCount,
		write_normals.begin(),
		[](const vr::RenderModel_Vertex_t &vertex) {
		return glm::vec3(vertex.vNormal.v[0],
			vertex.vPosition.v[1], vertex.vPosition.v[2]);
	});

	type::vec2_array texcoords(pModel->unVertexCount);
	auto write_texcoords(type::write(texcoords));
	std::transform(pModel->rVertexData,
		pModel->rVertexData + pModel->unVertexCount,
		write_texcoords.begin(),
		[](const vr::RenderModel_Vertex_t &vertex) {
		return glm::vec2(vertex.rfTextureCoord[0],
			vertex.rfTextureCoord[1]);
	});

	vcc::image::image_type image(vcc::image::create(
		vcc::internal::get_parent(*queue), 0,
		VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
		VkExtent3D{ pTexture->unWidth, pTexture->unHeight, 1 },
		1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, {},
		VK_IMAGE_LAYOUT_UNDEFINED));
	vcc::memory::bind(vcc::internal::get_parent(*queue),
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, image);
	vcc::image::copy_to_linear_image(VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VkExtent2D{ pTexture->unWidth, pTexture->unHeight },
		pTexture->rubTextureMapData, 4, 4 * pTexture->unWidth, image);

	render_models->FreeRenderModel(pModel);
	render_models->FreeTexture(pTexture);
	return vr_model{ std::move(indices), std::move(vertices),
		std::move(normals), std::move(texcoords), std::move(image) };
}
