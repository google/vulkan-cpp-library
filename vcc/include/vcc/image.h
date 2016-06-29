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
#ifndef IMAGE_H_
#define IMAGE_H_

#include <vcc/device.h>

namespace vcc {
namespace image {

	struct image_type;
}  // namespace image

namespace memory {
struct memory_type;
}  // namespace memory

namespace swapchain {
struct swapchain_type;
VCC_LIBRARY std::vector<image::image_type> get_images(
	swapchain_type &swapchain);
}  // namespace memory

namespace image {

struct image_type
	: public internal::movable_conditional_destructible_with_parent_and_memory<
		VkImage, device::device_type, memory::memory_type, vkDestroyImage> {
	friend VCC_LIBRARY image_type create(
		const type::supplier<device::device_type> &device,
		VkImageCreateFlags flags, VkImageType imageType, VkFormat format,
		const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
		VkSampleCountFlags samples, VkImageTiling tiling, VkImageUsageFlags usage,
		VkSharingMode sharingMode, const std::vector<uint32_t> &queueFamilyIndices,
		VkImageLayout initialLayout);
	friend VCC_LIBRARY std::vector<image::image_type> swapchain::get_images(swapchain::swapchain_type &swapchain);
	friend VkImageType get_type(const image_type &image);
	friend VkImageType get_type(const image_type &image);
	friend VkFormat get_format(const image_type &image);
	friend uint32_t get_mip_levels(const image_type &image);
	friend uint32_t get_array_layers(const image_type &image);

	image_type() = default;
	image_type(const image_type&) = delete;
	image_type(image_type &&instance) = default;
	image_type &operator=(const image_type&) = delete;
	image_type &operator=(image_type&&copy) = default;

private:
	image_type(VkImage instance, const type::supplier<device::device_type> &parent,
		bool destructible, VkImageType type, VkFormat format, uint32_t mipLevels,
		uint32_t arrayLayers)
		:  internal::movable_conditional_destructible_with_parent_and_memory<
			VkImage, device::device_type, memory::memory_type, vkDestroyImage>(
			instance, parent, destructible),
		  type(type), format(format), mipLevels(mipLevels),
		  arrayLayers(arrayLayers) {}

	VkImageType type;
	VkFormat format;
	uint32_t mipLevels, arrayLayers;
};

VCC_LIBRARY image_type create(
	const type::supplier<device::device_type> &device,
	VkImageCreateFlags flags, VkImageType imageType, VkFormat format,
	const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
	VkSampleCountFlags samples, VkImageTiling tiling, VkImageUsageFlags usage,
	VkSharingMode sharingMode, const std::vector<uint32_t> &queueFamilyIndices,
	VkImageLayout initialLayout);

inline VkImageType get_type(const image_type &image) {
	return image.type;
}

inline VkFormat get_format(const image_type &image) {
	return image.format;
}

inline uint32_t get_mip_levels(const image_type &image) {
	return image.mipLevels;
}

inline uint32_t get_array_layers(const image_type &image) {
	return image.arrayLayers;
}

VCC_LIBRARY VkSubresourceLayout get_subresource_layout(image_type &image,
	const VkImageSubresource &subresource);

/*
 * Copy to the mipmap level 0, array 0 of an VK_IMAGE_TILING_LINEAR image.
 */
VCC_LIBRARY void copy_to_linear_image(VkFormat format,
	VkImageAspectFlags aspect_mask, VkExtent2D extent, const void *source,
	std::size_t block_size, std::size_t row_pitch,
	image::image_type &target_image);

}  // namespace image
}  // namespace vcc

#endif /* IMAGE_H_ */
