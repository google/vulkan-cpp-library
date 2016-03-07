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
#ifndef IMAGE_VIEW_H_
#define IMAGE_VIEW_H_

#include <vcc/device.h>
#include <vcc/image.h>

namespace vcc {
namespace image_view {

struct image_view_type
	: public internal::movable_destructible_with_parent<VkImageView,
		device::device_type, vkDestroyImageView> {
	friend VCC_LIBRARY image_view_type create(
		const type::supplier<image::image_type> &image,
		VkImageViewType viewType, VkFormat format,
		const VkComponentMapping &components,
		const VkImageSubresourceRange &subresourceRange);

	friend VCC_LIBRARY image_view_type create(
		const type::supplier<image::image_type> &image,
		const VkImageSubresourceRange &subresourceRange);

	image_view_type(const image_view_type &) = delete;
	image_view_type(image_view_type &&copy) = default;
	image_view_type& operator=(const image_view_type&) = delete;
	image_view_type& operator=(image_view_type&&copy) = default;

private:
	image_view_type(VkImageView instance,
		const type::supplier<device::device_type> &parent,
		const type::supplier<image::image_type> &image)
		: internal::movable_destructible_with_parent<VkImageView,
		device::device_type, vkDestroyImageView>(instance, parent),
		image(image) {}
	type::supplier<image::image_type> image;
};

VCC_LIBRARY image_view_type create(
	const type::supplier<image::image_type> &image,
	VkImageViewType viewType, VkFormat format,
	const VkComponentMapping &components,
	const VkImageSubresourceRange &subresourceRange);

VCC_LIBRARY image_view_type create(
	const type::supplier<image::image_type> &image,
	const VkImageSubresourceRange &subresourceRange);

}  // namespace image_view
}  // namespace vcc

#endif /* IMAGE_VIEW_H_ */
