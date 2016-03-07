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
#ifndef SWAP_CHAIN_H_
#define SWAP_CHAIN_H_

#include <chrono>
#include <vcc/device.h>
#include <vcc/fence.h>
#include <vcc/image.h>
#include <vcc/semaphore.h>
#include <vcc/surface.h>

namespace vcc {
namespace swapchain {

struct create_info_type {
	type::supplier<surface::surface_type> surface;
	uint32_t minImageCount;
	VkFormat imageFormat;
	VkColorSpaceKHR imageColorSpace;
	VkExtent2D imageExtent;
	uint32_t imageArrayLayers;
	VkImageUsageFlags imageUsage;
	VkSharingMode imageSharingMode;
	std::vector<uint32_t> queueFamilyIndices;
	VkSurfaceTransformFlagBitsKHR preTransform;
	VkCompositeAlphaFlagsKHR compositeAlpha;
	VkPresentModeKHR presentMode;
	VkBool32 clipped;
	type::supplier<swapchain_type> oldSwapchain;
};

struct swapchain_type
	: public internal::movable_destructible_with_parent<VkSwapchainKHR,
		device::device_type, vkDestroySwapchainKHR> {
	friend VCC_LIBRARY swapchain_type create(
		const type::supplier<device::device_type> &device,
		const create_info_type &swapchain_create_info);
	friend VkFormat get_format(swapchain_type &swapchain);

	swapchain_type() = default;
	swapchain_type(const swapchain_type &) = delete;
	swapchain_type(swapchain_type &&copy) = default;
	swapchain_type &operator=(const swapchain_type &) = delete;
	swapchain_type &operator=(swapchain_type &&copy) = default;

private:
	swapchain_type(VkSwapchainKHR instance,
		const type::supplier<device::device_type> &parent, VkFormat format)
		: internal::movable_destructible_with_parent<VkSwapchainKHR,
			device::device_type, vkDestroySwapchainKHR>(instance, parent),
		  format(format) {}

	VkFormat format;
};

VCC_LIBRARY swapchain_type create(
	const type::supplier<device::device_type> &device,
	const create_info_type &swapchain_create_info);

VCC_LIBRARY std::vector<image::image_type> get_images(
	swapchain_type &swapchain);

VCC_LIBRARY std::tuple<VkResult, uint32_t> acquire_next_image(
	swapchain_type &swapchain, std::chrono::nanoseconds timeout,
	const semaphore::semaphore_type &semaphore,
	const fence::fence_type &fence);
VCC_LIBRARY std::tuple<VkResult, uint32_t> acquire_next_image(
	swapchain_type &swapchain, std::chrono::nanoseconds timeout,
	const semaphore::semaphore_type &semaphore);
VCC_LIBRARY std::tuple<VkResult, uint32_t> acquire_next_image(
	swapchain_type &swapchain, std::chrono::nanoseconds timeout,
	const fence::fence_type &fence);
VCC_LIBRARY std::tuple<VkResult, uint32_t> acquire_next_image(
	swapchain_type &swapchain, const semaphore::semaphore_type &semaphore);

inline VkFormat get_format(swapchain_type &swapchain) {
	return swapchain.format;
}

}  // namespace swapchain
}  // namespace vcc


#endif /* SWAP_CHAIN_H_ */
