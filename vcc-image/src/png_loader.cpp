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
#include <png.h>
#include <vcc/image.h>
#include <vcc/internal/loader.h>
#include <vcc/memory.h>
#include <vcc/queue.h>

namespace vcc {
namespace image {
namespace internal {

void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	std::istream &stream(*(std::istream *) png_get_io_ptr(png_ptr));
	stream.read((char *)data, length);
	if (stream.eof()) {
		png_error(png_ptr, "EOF");
	}
}

bool png_loader_type::can_load(std::istream &stream) {
	const unsigned char signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
	unsigned char buffer[sizeof(signature)];
	stream.read((char *) buffer, sizeof(buffer));
	stream.seekg(stream.beg);
	return std::equal(buffer, buffer + sizeof(signature), signature);
}

image::image_type png_loader_type::load(
	const type::supplier<vcc::queue::queue_type> &queue,
	VkImageCreateFlags flags,
	VkImageUsageFlags usage,
	VkFormatFeatureFlags feature_flags,
	VkSharingMode sharingMode,
	const std::vector<uint32_t> &queueFamilyIndices,
	std::istream &stream) {
	std::unique_ptr<png_struct, std::function<void(png_structp)>> png_ptr(
		png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL), [](png_structp png_ptr) {
			png_destroy_read_struct(&png_ptr, NULL, NULL);
		});
	assert(png_ptr);
	std::unique_ptr<png_info, std::function<void(png_infop)>> info_ptr(
		png_create_info_struct(png_ptr.get()), [&png_ptr](png_infop info_ptr) {
			png_destroy_info_struct(png_ptr.get(), &info_ptr);
		});
	assert(info_ptr);

	if (setjmp(png_jmpbuf(png_ptr.get()))) {
		// TODO(gardell): Not sure signal handlers allow throw?
		throw std::runtime_error("reading png failed");
	}
	png_set_read_fn(png_ptr.get(), &stream, &user_read_data);

	unsigned int sig_read = 0;
	png_set_sig_bytes(png_ptr.get(), sig_read);
	png_read_png(png_ptr.get(), info_ptr.get(),
		PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING
		| PNG_TRANSFORM_EXPAND, NULL);

	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_get_IHDR(png_ptr.get(), info_ptr.get(), &width, &height, &bit_depth, &color_type,
		&interlace_type, NULL, NULL);

	VkFormat format;
	switch (color_type) {
	case PNG_COLOR_TYPE_RGBA:
		format = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case PNG_COLOR_TYPE_RGB:
		format = VK_FORMAT_R8G8B8_UNORM;
		break;
		// Probably never happens since PNG_TRANSFORM_EXPAND, should use low level api for more control.
	case PNG_COLOR_TYPE_GRAY:
		format = VK_FORMAT_R8_UNORM;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		format = VK_FORMAT_R8G8_UNORM;
		break;
	default:
		assert(!"Unsupported png format");
		break;
	}

	png_size_t row_bytes = png_get_rowbytes(png_ptr.get(), info_ptr.get());
	std::string data(row_bytes * height, 0);

	png_bytepp row_pointers = png_get_rows(png_ptr.get(), info_ptr.get());

	const bool flip_y(true);
	const png_uint_32 row_start(flip_y ? height - 1 : 0), row_increment(flip_y ? -1 : 1);
	for (png_uint_32 i = 0; i < height; i++) {
		std::copy_n(row_pointers[i], row_bytes, data.begin() + (row_bytes * (row_start + row_increment * i)));
	}

	const VkExtent3D extent{ width, height, 1 };
	const type::supplier<device::device_type> device(vcc::internal::get_parent(*queue));
	image::image_type image(image::create(device, 0, VK_IMAGE_TYPE_2D,
		format, extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR,
		usage, sharingMode, queueFamilyIndices, VK_IMAGE_LAYOUT_UNDEFINED));
	memory::bind(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, image);
	copy_to_image(*queue, device::get_physical_device(*device), format, VK_IMAGE_ASPECT_COLOR_BIT,
		VkExtent2D{ extent.width, extent.height }, (uint8_t *)data.c_str(),
		bytes_per_pixel(format), image);
	return std::move(image);
}

}  // namespace internal
}  // namespace image
}  // namespace vcc
