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
#ifndef _OPENVR_VR_H_
#define _OPENVR_VR_H_

#include <array>
#include <glm/glm.hpp>
#include <openvr.h>
#include <string>
#include <type/types.h>
#include <vcc/image.h>
#include <vcc/image_view.h>
#include <vcc/semaphore.h>

struct vr_model {
	type::ushort_array indices;
	type::vec3_array vertices, normals;
	type::vec2_array texcoords;
	vcc::image::image_type image;
};

struct hmd_type {
	friend struct vr_type;

	hmd_type();
	hmd_type(const hmd_type &) = delete;
	hmd_type(hmd_type &&) = default;
	hmd_type &operator=(hmd_type &&) = delete;
	hmd_type &operator=(const hmd_type &) = default;

	std::vector<std::string> get_vulkan_instance_extensions_required() const;
	std::vector<std::string> get_vulkan_device_extensions_required(
		VkPhysicalDevice physical_device) const;

	vr::IVRSystem &get_hmd() const {
		return *hmd;
	}

private:
	vr::IVRSystem *hmd;
};

struct vr_type {
	typedef std::function<void(const vr::VREvent_t &)> event_callback_type;
	typedef std::function<void(const std::array<glm::mat4x3, vr::k_unMaxTrackedDeviceCount> &)>
		draw_callback_type;

	vr_type(hmd_type &&hmd,
		const type::supplier<const vcc::instance::instance_type> &instance,
		const type::supplier<const vcc::queue::queue_type> &queue);
	~vr_type();

	int run(const draw_callback_type &draw_callback,
		const event_callback_type &event_callback);

	VkExtent2D get_recommended_render_target_size() const;

	glm::mat4 get_projection_matrix(vr::Hmd_Eye nEye, float near_z,
		float far_z) const;

	glm::mat4 get_head_to_eye_transform(vr::Hmd_Eye nEye) const;

	std::string get_string_tracked_device_property(
		vr::TrackedDeviceIndex_t unDeviceIndex,
		vr::ETrackedDeviceProperty prop) const;

	vcc::image::image_type &get_image() {
		return image;
	}

	vcc::image_view::image_view_type &get_image_view() {
		return image_view;
	}

	VkViewport get_viewport(int index, float minDepth, float maxDepth) const {
		const uint32_t width(extent.width / 2);
		return VkViewport{ float(width * index), 0, float(width),
			float(extent.height), minDepth, maxDepth };
	}

	vr_model load_model(const char *model_name) const;

	hmd_type hmd;
private:
	type::supplier<const vcc::instance::instance_type> instance;
	type::supplier<const vcc::queue::queue_type> queue;
	vcc::image::image_type image;
	vcc::image_view::image_view_type image_view;
	VkExtent2D extent;
};

#endif  // _OPENVR_VR_H_
