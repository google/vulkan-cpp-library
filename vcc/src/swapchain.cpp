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
#include <cstdint>
#include <vcc/swapchain.h>

#ifndef UINT64_MAX // TODO(gardell): Should not be needed
#define UINT64_MAX       (UINT64_C(18446744073709551615))
#define UINT64_C(c)     c ## ULL
#endif // UINT64_MAX

namespace vcc {
namespace swapchain {

swapchain_type create(const type::supplier<device::device_type> &device, const create_info_type &create_info) {
	VkSwapchainCreateInfoKHR create = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, NULL, 0 };
	create.minImageCount = create_info.minImageCount;
	create.imageFormat = create_info.imageFormat;
	create.imageColorSpace = create_info.imageColorSpace;
	create.imageExtent = create_info.imageExtent;
	create.imageArrayLayers = create_info.imageArrayLayers;
	create.imageUsage = create_info.imageUsage;
	create.imageSharingMode = create_info.imageSharingMode;
	create.pQueueFamilyIndices = create_info.queueFamilyIndices.data();
	create.preTransform = create_info.preTransform;
	create.compositeAlpha = (VkCompositeAlphaFlagBitsKHR) create_info.compositeAlpha;
	create.presentMode = create_info.presentMode;
	create.clipped = create_info.clipped;
	if (create_info.oldSwapchain) {
		create.oldSwapchain = internal::get_instance(*create_info.oldSwapchain);
	} else {
		create.oldSwapchain = VK_NULL_HANDLE;
	}
	VkSwapchainKHR swapchain;
	{
		if (create_info.oldSwapchain) {
			std::lock(internal::get_mutex(*create_info.surface), internal::get_mutex(*create_info.oldSwapchain));
			std::lock_guard<std::mutex> surface_lock(
				internal::get_mutex(*create_info.surface), std::adopt_lock);
			std::lock_guard<std::mutex> old_swapchain_lock(
				internal::get_mutex(*create_info.oldSwapchain), std::adopt_lock);
			create.surface = internal::get_instance(*create_info.surface);
			VKCHECK(vkCreateSwapchainKHR(internal::get_instance(*device), &create,
				NULL, &swapchain));
		} else {
			std::lock_guard<std::mutex> surface_lock(
				internal::get_mutex(*create_info.surface));
			create.surface = internal::get_instance(*create_info.surface);
			VKCHECK(vkCreateSwapchainKHR(internal::get_instance(*device), &create,
				NULL, &swapchain));
		}
	}
	return swapchain_type(swapchain, device, create_info.imageFormat);
}

std::vector<image::image_type> get_images(swapchain_type &swapchain) {
	uint32_t count;
	VKCHECK(vkGetSwapchainImagesKHR(
		internal::get_instance(*internal::get_parent(swapchain)),
		internal::get_instance(swapchain), &count, NULL));
	std::vector<VkImage> images(count);
	VKCHECK(vkGetSwapchainImagesKHR(internal::get_instance(*internal::get_parent(swapchain)), internal::get_instance(swapchain), &count, &images.front()));
	std::vector<image::image_type> converted_images;
	converted_images.reserve(images.size());
	for (VkImage image : images) {
		converted_images.push_back(image::image_type(image, internal::get_parent(swapchain), false, VK_IMAGE_TYPE_2D, get_format(swapchain), 1, 1));
	}
	return std::move(converted_images);
}

std::tuple<VkResult, uint32_t> acquire_next_image(swapchain_type &swapchain,
		std::chrono::nanoseconds timeout,
		const semaphore::semaphore_type &semaphore,
		const fence::fence_type &fence) {
	uint32_t image_index;
	std::lock(internal::get_mutex(swapchain), internal::get_mutex(semaphore));
	std::lock_guard<std::mutex> swapchain_lock(internal::get_mutex(swapchain), std::adopt_lock);
	std::lock_guard<std::mutex> semaphore_lock(internal::get_mutex(semaphore), std::adopt_lock);
	std::lock_guard<std::mutex> fence_lock(internal::get_mutex(fence), std::adopt_lock);
	const VkResult result(vkAcquireNextImageKHR(
		internal::get_instance(*internal::get_parent(swapchain)),
		internal::get_instance(swapchain), timeout.count(),
		internal::get_instance(semaphore), internal::get_instance(fence),
		&image_index));
	return std::make_tuple(result, image_index);
}

std::tuple<VkResult, uint32_t> acquire_next_image(swapchain_type &swapchain, std::chrono::nanoseconds timeout, const semaphore::semaphore_type &semaphore) {
	uint32_t image_index;
	const VkResult result(vkAcquireNextImageKHR(internal::get_instance(*internal::get_parent(swapchain)), internal::get_instance(swapchain), timeout.count(), internal::get_instance(semaphore), VK_NULL_HANDLE, &image_index));
	return std::make_tuple(result, image_index);
}

std::tuple<VkResult, uint32_t> acquire_next_image(swapchain_type &swapchain, std::chrono::nanoseconds timeout, const fence::fence_type &fence) {
	uint32_t image_index;
	const VkResult result(vkAcquireNextImageKHR(internal::get_instance(*internal::get_parent(swapchain)), internal::get_instance(swapchain), timeout.count(), VK_NULL_HANDLE, internal::get_instance(fence), &image_index));
	return std::make_tuple(result, image_index);
}

std::tuple<VkResult, uint32_t> acquire_next_image(swapchain_type &swapchain, const semaphore::semaphore_type &semaphore) {
	uint32_t image_index;
	const VkResult result(vkAcquireNextImageKHR(internal::get_instance(*internal::get_parent(swapchain)), internal::get_instance(swapchain), UINT64_MAX, internal::get_instance(semaphore), VK_NULL_HANDLE, &image_index));
	return std::make_tuple(result, image_index);
}

}  // namespace swapchain
}  // namespace vcc
