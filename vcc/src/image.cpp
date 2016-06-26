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
#include <vcc/image.h>

namespace vcc {
namespace image {

image_type create(const type::supplier<device::device_type> &device,
		VkImageCreateFlags flags,
		VkImageType imageType,
		VkFormat format,
		const VkExtent3D &extent,
		uint32_t mipLevels,
		uint32_t arrayLayers,
		VkSampleCountFlags samples,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkSharingMode sharingMode,
		const std::vector<uint32_t> &queueFamilyIndices,
		VkImageLayout initialLayout) {
 	VkImageCreateInfo create = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, NULL};
	create.flags = flags;
	create.imageType = imageType;
	create.format = format;
	create.extent = extent;
	create.mipLevels = mipLevels;
	create.arrayLayers = arrayLayers;
	create.samples = (VkSampleCountFlagBits) samples;
	create.tiling = tiling;
	create.usage = usage;
	create.sharingMode = sharingMode;
	create.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
	create.pQueueFamilyIndices = queueFamilyIndices.empty() ? NULL : &queueFamilyIndices.front();
	create.initialLayout = initialLayout;
	VkImage image;
	VKCHECK(vkCreateImage(internal::get_instance(*device), &create, NULL, &image));
	const VkDevice device_instance(internal::get_instance(*device));
	return image_type(image, device, true, imageType, format, mipLevels,
		arrayLayers);
}

VkSubresourceLayout get_subresource_layout(image_type &image,
		const VkImageSubresource &subresource) {
	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(
		internal::get_instance(*internal::get_parent(image)),
		internal::get_instance(image),
		&subresource, &layout);
	return layout;
}

}  // namespace image
}  // namespace vcc
