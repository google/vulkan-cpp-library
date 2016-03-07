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
#include <vcc/image_view.h>

namespace vcc {
namespace image_view {

image_view_type create(const type::supplier<image::image_type> &image,
	VkImageViewType viewType, VkFormat format,
	const VkComponentMapping &components,
	const VkImageSubresourceRange &subresourceRange) {
	VkImageViewCreateInfo create = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL, 0};
	create.image = vcc::internal::get_instance(*image);
	create.viewType = viewType;
	create.format = format;
	create.components = components;
	create.subresourceRange = subresourceRange;
	VkImageView image_view;
	VKCHECK(vkCreateImageView(internal::get_instance(*internal::get_parent(*image)), &create, NULL,
		&image_view));
	return image_view_type(image_view, internal::get_parent(*image), image);
}

VkImageViewType view_type_from_image_type(VkImageType image_type) {
	switch (image_type) {
	case VK_IMAGE_TYPE_1D:
		return VK_IMAGE_VIEW_TYPE_1D;
	case VK_IMAGE_TYPE_2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case VK_IMAGE_TYPE_3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	default:
		throw vcc_exception("Unknown VkImageType");
	}
}

image_view_type create(const type::supplier<image::image_type> &image,
	const VkImageSubresourceRange &subresourceRange) {
	return create(image, view_type_from_image_type(image::get_type(*image)),
		image::get_format(*image),
		{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
		subresourceRange);
}

}  // namespace image_view
}  // namespace vcc
