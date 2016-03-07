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
#ifndef _VCC_IMAGE_H_
#define _VCC_IMAGE_H_

#include <vcc/image.h>
#include <vcc/queue.h>

namespace vcc {
namespace image {

/*
 * Load an image from the given file.
 * Different loaders are supported, currently libpng and gli provide loading support.
 * In other words, .png, .ktx and .dds are supported.
 */
VCC_LIBRARY image::image_type create(
	const type::supplier<vcc::queue::queue_type> &queue,
	VkImageCreateFlags flags,
	VkImageUsageFlags usage,
	VkFormatFeatureFlags feature_flags,
	VkSharingMode sharingMode,
	const std::vector<uint32_t> &queueFamilyIndices,
	std::istream &stream);

// For debugging purposes only, prints the result of trying vkGetPhysicalDeviceFormatProperties with every VkFormat.
VCC_LIBRARY std::string dump_physical_device_format_properties(VkPhysicalDevice physical_device);

}  // namespace image
}  // namespace vcc

#endif // _VCC_IMAGE_H_