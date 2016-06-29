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
#include <gli/gli.hpp>
#include <sstream>
#include <vcc/command.h>
#include <vcc/image.h>
#include <vcc/internal/loader.h>
#include <vcc/memory.h>
#include <vcc/queue.h>

namespace vcc {
namespace image {
namespace internal {

bool gli_loader_type::can_load(std::istream &stream) {
	const unsigned char ktx_signature[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	const unsigned char dds_signature[] = "DDS ";
	unsigned char buffer[sizeof(ktx_signature)];
	stream.read((char *) buffer, sizeof(buffer));
	stream.seekg(stream.beg);
	return std::equal(buffer, buffer + sizeof(ktx_signature), ktx_signature)
		|| std::equal(buffer, buffer + sizeof(dds_signature), dds_signature);
}

VkImageType convert_type(gli::target target);
VkExtent3D convert_extent(const gli::extent3d &extent);
VkFormat convert_format(gli::format format);

// Note: technically, we could end up with a remainder,
// but for example BC1 is required to have dimensions in multiples of ifs block size.
glm::ivec3 compressed_extent(const gli::texture &texture) {
	const glm::ivec3 block_extent(gli::block_extent(texture.format()));
	const gli::extent3d extent(texture.extent());
	return glm::ivec3(extent / block_extent);
}

image::image_type gli_loader_type::load(
	const type::supplier<vcc::queue::queue_type> &queue,
	VkImageCreateFlags flags,
	VkImageUsageFlags usage,
	VkFormatFeatureFlags feature_flags,
	VkSharingMode sharingMode,
	const std::vector<uint32_t> &queueFamilyIndices,
	std::istream &stream) {
	std::stringstream ss;
	ss << stream.rdbuf();
	const std::string data(ss.str());
	gli::texture texture(gli::load(data.c_str(), data.size()));
	if (texture.empty()) {
		VCC_PRINT("failed to load texture");
		throw vcc_exception("failed to load texture");
	}
	const type::supplier<device::device_type> device(
		vcc::internal::get_parent(*queue));
	const VkPhysicalDevice physical_device(device::get_physical_device(*device));
	const VkFormat format(convert_format(texture.format()));
	const VkImageType type(convert_type(texture.target()));
	const VkExtent3D extent(convert_extent(texture.extent()));
	const uint32_t face_total(static_cast<uint32_t>(
		texture.layers() * texture.faces()));
	// TODO(gardell): Support different aspect masks?
	const VkImageAspectFlags aspect_mask(VK_IMAGE_ASPECT_COLOR_BIT);

	VkImageFormatProperties imageFormatProperties;
	VKCHECK(vkGetPhysicalDeviceImageFormatProperties(physical_device, format,
		type, VK_IMAGE_TILING_OPTIMAL, usage, flags, &imageFormatProperties));

	image::image_type image(image::create(device,
		flags, type, format, extent, static_cast<uint32_t>(texture.levels()),
		face_total, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
		usage, sharingMode, queueFamilyIndices, VK_IMAGE_LAYOUT_UNDEFINED));
	memory::bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

	command_pool::command_pool_type command_pool(command_pool::create(
		device, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queue::get_family_index(*queue)));
	command_buffer::command_buffer_type command_buffer(std::move(command_buffer::allocate(
		device, std::ref(command_pool), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));
	image::image_type staging_image(image::create(device, 0, VK_IMAGE_TYPE_2D,
		format, VkExtent3D{ extent.width, extent.height, 1 }, 1, 1,
		VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE,
		{ queue::get_family_index(*queue) }, VK_IMAGE_LAYOUT_UNDEFINED));
	memory::bind(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, staging_image);
	for (std::size_t layer = 0; layer < texture.layers(); ++layer) {
		for (std::size_t face = 0; face < texture.faces(); ++face) {
			for (std::size_t level = 0; level < texture.levels(); ++level) {
				for (uint32_t z = 0; z < extent.depth; ++z) {
					const std::size_t layer_index(layer * texture.faces()
						+ face);
					const glm::ivec3 extent(glm::ivec3(texture.extent(level)));
					const glm::ivec3 copy_extent(
						gli::is_compressed(texture.format())
						? compressed_extent(texture) : extent);
					const std::size_t block_size(gli::block_size(texture.format()));
					copy_to_linear_image(format, aspect_mask,
						VkExtent2D{ uint32_t(copy_extent.x), uint32_t(copy_extent.y) },
						texture.data(layer, face, level), block_size,
						block_size * copy_extent.x, staging_image);

					command_buffer::compile(command_buffer,
						VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, VK_FALSE, 0, 0,
						command::pipeline_barrier(
							VK_PIPELINE_STAGE_HOST_BIT,
							VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
							{},{},
							{
								command::image_memory_barrier{
									VK_ACCESS_HOST_WRITE_BIT,
									VK_ACCESS_TRANSFER_READ_BIT,
									VK_IMAGE_LAYOUT_UNDEFINED,
									VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
									VK_QUEUE_FAMILY_IGNORED,
									VK_QUEUE_FAMILY_IGNORED,
									std::ref(staging_image),
									{ aspect_mask, 0, 1, 0, 1 } },
								command::image_memory_barrier{
									0, VK_ACCESS_TRANSFER_READ_BIT,
									VK_IMAGE_LAYOUT_GENERAL,
									VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
									VK_QUEUE_FAMILY_IGNORED,
									VK_QUEUE_FAMILY_IGNORED,
									std::ref(image),
									{ aspect_mask, uint32_t(level), 1,
									uint32_t(layer_index), 1 } }
							}),
						command::copy_image{ std::ref(staging_image),
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						std::ref(image), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						{ VkImageCopy{
							{ aspect_mask, 0, 0, 1 },
							{ 0, 0, 0 },
							{ aspect_mask, uint32_t(level),
								uint32_t(layer_index), 1 },
							{ 0, 0, int32_t(z) },
							{ uint32_t(extent.x), uint32_t(extent.y), 1 }
					} } });
					queue::submit(*queue, {}, { std::ref(command_buffer) },
						{});
				}
			}
		}
	}
	return std::move(image);
}

VkImageType convert_type(gli::target target) {
	switch (target) {
	case gli::TARGET_1D:
	case gli::TARGET_1D_ARRAY:
		return VK_IMAGE_TYPE_1D;
	case gli::TARGET_2D:
	case gli::TARGET_2D_ARRAY:
	case gli::TARGET_RECT:
	case gli::TARGET_RECT_ARRAY:
	case gli::TARGET_CUBE:
	case gli::TARGET_CUBE_ARRAY:
		return VK_IMAGE_TYPE_2D;
	case gli::TARGET_3D:
		return VK_IMAGE_TYPE_3D;
	default:
		throw vcc_exception("Unknown target");
	}
}

VkExtent3D convert_extent(const gli::extent3d &extent) {
	return VkExtent3D{ uint32_t(extent.x), uint32_t(extent.y), uint32_t(extent.z) };
}

VkFormat convert_format(gli::format format) {
	switch (format) {
	case gli::FORMAT_RG4_UNORM_PACK8:
		return VK_FORMAT_R4G4_UNORM_PACK8;
	case gli::FORMAT_RGBA4_UNORM_PACK16:
		return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
	case gli::FORMAT_BGRA4_UNORM_PACK16:
		return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
	case gli::FORMAT_R5G6B5_UNORM_PACK16:
		return VK_FORMAT_R5G6B5_UNORM_PACK16;
	case gli::FORMAT_B5G6R5_UNORM_PACK16:
		return VK_FORMAT_B5G6R5_UNORM_PACK16;
	case gli::FORMAT_RGB5A1_UNORM_PACK16:
		return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
	case gli::FORMAT_BGR5A1_UNORM_PACK16:
		return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
	case gli::FORMAT_A1RGB5_UNORM_PACK16:
		return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
	case gli::FORMAT_R8_UNORM_PACK8:
		return VK_FORMAT_R8_UNORM;
	case gli::FORMAT_R8_SNORM_PACK8:
		return VK_FORMAT_R8_SNORM;
	case gli::FORMAT_R8_USCALED_PACK8:
		return VK_FORMAT_R8_USCALED;
	case gli::FORMAT_R8_SSCALED_PACK8:
		return VK_FORMAT_R8_SSCALED;
	case gli::FORMAT_R8_UINT_PACK8:
		return VK_FORMAT_R8_UINT;
	case gli::FORMAT_R8_SINT_PACK8:
		return VK_FORMAT_R8_SINT;
	case gli::FORMAT_R8_SRGB_PACK8:
		return VK_FORMAT_R8_SRGB;
	case gli::FORMAT_RG8_UNORM_PACK8:
		return VK_FORMAT_R8G8_UNORM;
	case gli::FORMAT_RG8_SNORM_PACK8:
		return VK_FORMAT_R8G8_SNORM;
	case gli::FORMAT_RG8_USCALED_PACK8:
		return VK_FORMAT_R8G8_USCALED;
	case gli::FORMAT_RG8_SSCALED_PACK8:
		return VK_FORMAT_R8G8_SSCALED;
	case gli::FORMAT_RG8_UINT_PACK8:
		return VK_FORMAT_R8G8_UINT;
	case gli::FORMAT_RG8_SINT_PACK8:
		return VK_FORMAT_R8G8_SINT;
	case gli::FORMAT_RG8_SRGB_PACK8:
		return VK_FORMAT_R8G8_SRGB;
	case gli::FORMAT_RGB8_UNORM_PACK8:
		return VK_FORMAT_R8G8B8_UNORM;
	case gli::FORMAT_RGB8_SNORM_PACK8:
		return VK_FORMAT_R8G8B8_SNORM;
	case gli::FORMAT_RGB8_USCALED_PACK8:
		return VK_FORMAT_R8G8B8_USCALED;
	case gli::FORMAT_RGB8_SSCALED_PACK8:
		return VK_FORMAT_R8G8B8_SSCALED;
	case gli::FORMAT_RGB8_UINT_PACK8:
		return VK_FORMAT_R8G8B8_UINT;
	case gli::FORMAT_RGB8_SINT_PACK8:
		return VK_FORMAT_R8G8B8_SINT;
	case gli::FORMAT_RGB8_SRGB_PACK8:
		return VK_FORMAT_R8G8B8_SRGB;
	case gli::FORMAT_BGR8_UNORM_PACK8:
		return VK_FORMAT_B8G8R8_UNORM;
	case gli::FORMAT_BGR8_SNORM_PACK8:
		return VK_FORMAT_B8G8R8_SNORM;
	case gli::FORMAT_BGR8_USCALED_PACK8:
		return VK_FORMAT_B8G8R8_USCALED;
	case gli::FORMAT_BGR8_SSCALED_PACK8:
		return VK_FORMAT_B8G8R8_SSCALED;
	case gli::FORMAT_BGR8_UINT_PACK8:
		return VK_FORMAT_B8G8R8_UINT;
	case gli::FORMAT_BGR8_SINT_PACK8:
		return VK_FORMAT_B8G8R8_SINT;
	case gli::FORMAT_BGR8_SRGB_PACK8:
		return VK_FORMAT_B8G8R8_SRGB;
	case gli::FORMAT_RGBA8_UNORM_PACK8:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case gli::FORMAT_RGBA8_SNORM_PACK8:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case gli::FORMAT_RGBA8_USCALED_PACK8:
		return VK_FORMAT_R8G8B8A8_USCALED;
	case gli::FORMAT_RGBA8_SSCALED_PACK8:
		return VK_FORMAT_R8G8B8A8_SSCALED;
	case gli::FORMAT_RGBA8_UINT_PACK8:
		return VK_FORMAT_R8G8B8A8_UINT;
	case gli::FORMAT_RGBA8_SINT_PACK8:
		return VK_FORMAT_R8G8B8A8_SINT;
	case gli::FORMAT_RGBA8_SRGB_PACK8:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case gli::FORMAT_BGRA8_UNORM_PACK8:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case gli::FORMAT_BGRA8_SNORM_PACK8:
		return VK_FORMAT_B8G8R8A8_SNORM;
	case gli::FORMAT_BGRA8_USCALED_PACK8:
		return VK_FORMAT_B8G8R8A8_USCALED;
	case gli::FORMAT_BGRA8_SSCALED_PACK8:
		return VK_FORMAT_B8G8R8A8_SSCALED;
	case gli::FORMAT_BGRA8_UINT_PACK8:
		return VK_FORMAT_B8G8R8A8_UINT;
	case gli::FORMAT_BGRA8_SINT_PACK8:
		return VK_FORMAT_B8G8R8A8_SINT;
	case gli::FORMAT_BGRA8_SRGB_PACK8:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case gli::FORMAT_RGBA8_UNORM_PACK32:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case gli::FORMAT_RGBA8_SNORM_PACK32:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case gli::FORMAT_RGBA8_USCALED_PACK32:
		return VK_FORMAT_R8G8B8A8_USCALED;
	case gli::FORMAT_RGBA8_SSCALED_PACK32:
		return VK_FORMAT_R8G8B8A8_SSCALED;
	case gli::FORMAT_RGBA8_UINT_PACK32:
		return VK_FORMAT_R8G8B8A8_UINT;
	case gli::FORMAT_RGBA8_SINT_PACK32:
		return VK_FORMAT_R8G8B8A8_SINT;
	case gli::FORMAT_RGBA8_SRGB_PACK32:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case gli::FORMAT_RGB10A2_UNORM_PACK32:
		throw vcc_exception("Unsupported FORMAT_RGB10A2_UNORM_PACK32");
	case gli::FORMAT_RGB10A2_SNORM_PACK32:
		throw vcc_exception("Unsupported FORMAT_RGB10A2_SNORM_PACK32");
	case gli::FORMAT_RGB10A2_USCALED_PACK32:
		throw vcc_exception("Unsupported FORMAT_RGB10A2_USCALED_PACK32");
	case gli::FORMAT_RGB10A2_SSCALED_PACK32:
		throw vcc_exception("Unsupported FORMAT_RGB10A2_SSCALED_PACK32");
	case gli::FORMAT_RGB10A2_UINT_PACK32:
		throw vcc_exception("Unsupported FORMAT_RGB10A2_UINT_PACK32");
	case gli::FORMAT_RGB10A2_SINT_PACK32:
		throw vcc_exception("Unsupported FORMAT_RGB10A2_SINT_PACK32");
	case gli::FORMAT_BGR10A2_UNORM_PACK32:
		throw vcc_exception("Unsupported FORMAT_BGR10A2_UNORM_PACK32");
	case gli::FORMAT_BGR10A2_SNORM_PACK32:
		throw vcc_exception("Unsupported FORMAT_BGR10A2_SNORM_PACK32");
	case gli::FORMAT_BGR10A2_USCALED_PACK32:
		throw vcc_exception("Unsupported FORMAT_BGR10A2_USCALED_PACK32");
	case gli::FORMAT_BGR10A2_SSCALED_PACK32:
		throw vcc_exception("Unsupported FORMAT_BGR10A2_SSCALED_PACK32");
	case gli::FORMAT_BGR10A2_UINT_PACK32:
		throw vcc_exception("Unsupported FORMAT_BGR10A2_UINT_PACK32");
	case gli::FORMAT_BGR10A2_SINT_PACK32:
		throw vcc_exception("Unsupported FORMAT_BGR10A2_SINT_PACK32");
	case gli::FORMAT_R16_UNORM_PACK16:
		return VK_FORMAT_R16_UNORM;
	case gli::FORMAT_R16_SNORM_PACK16:
		return VK_FORMAT_R16_SNORM;
	case gli::FORMAT_R16_USCALED_PACK16:
		return VK_FORMAT_R16_USCALED;
	case gli::FORMAT_R16_SSCALED_PACK16:
		return VK_FORMAT_R16_SSCALED;
	case gli::FORMAT_R16_UINT_PACK16:
		return VK_FORMAT_R16_UINT;
	case gli::FORMAT_R16_SINT_PACK16:
		return VK_FORMAT_R16_SINT;
	case gli::FORMAT_R16_SFLOAT_PACK16:
		return VK_FORMAT_R16_SFLOAT;
	case gli::FORMAT_RG16_UNORM_PACK16:
		return VK_FORMAT_R16_UNORM;
	case gli::FORMAT_RG16_SNORM_PACK16:
		return VK_FORMAT_R16_SNORM;
	case gli::FORMAT_RG16_USCALED_PACK16:
		return VK_FORMAT_R16G16_USCALED;
	case gli::FORMAT_RG16_SSCALED_PACK16:
		return VK_FORMAT_R16G16_SSCALED;
	case gli::FORMAT_RG16_UINT_PACK16:
		return VK_FORMAT_R16G16_UINT;
	case gli::FORMAT_RG16_SINT_PACK16:
		return VK_FORMAT_R16G16_SINT;
	case gli::FORMAT_RG16_SFLOAT_PACK16:
		return VK_FORMAT_R16G16_SFLOAT;
	case gli::FORMAT_RGB16_UNORM_PACK16:
		return VK_FORMAT_R16G16B16_UNORM;
	case gli::FORMAT_RGB16_SNORM_PACK16:
		return VK_FORMAT_R16G16B16_SNORM;
	case gli::FORMAT_RGB16_USCALED_PACK16:
		return VK_FORMAT_R16G16B16_USCALED;
	case gli::FORMAT_RGB16_SSCALED_PACK16:
		return VK_FORMAT_R16G16B16_SSCALED;
	case gli::FORMAT_RGB16_UINT_PACK16:
		return VK_FORMAT_R16G16B16_UINT;
	case gli::FORMAT_RGB16_SINT_PACK16:
		return VK_FORMAT_R16G16B16_SINT;
	case gli::FORMAT_RGB16_SFLOAT_PACK16:
		return VK_FORMAT_R16G16B16_SFLOAT;
	case gli::FORMAT_RGBA16_UNORM_PACK16:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case gli::FORMAT_RGBA16_SNORM_PACK16:
		return VK_FORMAT_R16G16B16_SNORM;
	case gli::FORMAT_RGBA16_USCALED_PACK16:
		return VK_FORMAT_R16G16B16A16_USCALED;
	case gli::FORMAT_RGBA16_SSCALED_PACK16:
		return VK_FORMAT_R16G16B16A16_SSCALED;
	case gli::FORMAT_RGBA16_UINT_PACK16:
		return VK_FORMAT_R16G16B16A16_UINT;
	case gli::FORMAT_RGBA16_SINT_PACK16:
		return VK_FORMAT_R16G16B16A16_SINT;
	case gli::FORMAT_RGBA16_SFLOAT_PACK16:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case gli::FORMAT_R32_UINT_PACK32:
		return VK_FORMAT_R32_UINT;
	case gli::FORMAT_R32_SINT_PACK32:
		return VK_FORMAT_R32_SINT;
	case gli::FORMAT_R32_SFLOAT_PACK32:
		return VK_FORMAT_R32_SFLOAT;
	case gli::FORMAT_RG32_UINT_PACK32:
		return VK_FORMAT_R32G32_UINT;
	case gli::FORMAT_RG32_SINT_PACK32:
		return VK_FORMAT_R32G32_SINT;
	case gli::FORMAT_RG32_SFLOAT_PACK32:
		return VK_FORMAT_R32G32_SFLOAT;
	case gli::FORMAT_RGB32_UINT_PACK32:
		return VK_FORMAT_R32G32B32_UINT;
	case gli::FORMAT_RGB32_SINT_PACK32:
		return VK_FORMAT_R32G32B32_SINT;
	case gli::FORMAT_RGB32_SFLOAT_PACK32:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case gli::FORMAT_RGBA32_UINT_PACK32:
		return VK_FORMAT_R32G32B32A32_UINT;
	case gli::FORMAT_RGBA32_SINT_PACK32:
		return VK_FORMAT_R32G32B32A32_SINT;
	case gli::FORMAT_RGBA32_SFLOAT_PACK32:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case gli::FORMAT_R64_UINT_PACK64:
		return VK_FORMAT_R64_UINT;
	case gli::FORMAT_R64_SINT_PACK64:
		return VK_FORMAT_R64_SINT;
	case gli::FORMAT_R64_SFLOAT_PACK64:
		return VK_FORMAT_R64_SFLOAT;
	case gli::FORMAT_RG64_UINT_PACK64:
		return VK_FORMAT_R64G64_UINT;
	case gli::FORMAT_RG64_SINT_PACK64:
		return VK_FORMAT_R64G64_SINT;
	case gli::FORMAT_RG64_SFLOAT_PACK64:
		return VK_FORMAT_R64G64_SFLOAT;
	case gli::FORMAT_RGB64_UINT_PACK64:
		return VK_FORMAT_R64G64B64_UINT;
	case gli::FORMAT_RGB64_SINT_PACK64:
		return VK_FORMAT_R64G64B64_SINT;
	case gli::FORMAT_RGB64_SFLOAT_PACK64:
		return VK_FORMAT_R64G64B64_SFLOAT;
	case gli::FORMAT_RGBA64_UINT_PACK64:
		return VK_FORMAT_R64G64B64A64_UINT;
	case gli::FORMAT_RGBA64_SINT_PACK64:
		return VK_FORMAT_R64G64B64A64_SINT;
	case gli::FORMAT_RGBA64_SFLOAT_PACK64:
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	case gli::FORMAT_RG11B10_UFLOAT_PACK32:
		throw vcc_exception("Unsupported format FORMAT_RG11B10_UFLOAT_PACK32");
	case gli::FORMAT_RGB9E5_UFLOAT_PACK32:
		throw vcc_exception("Unsupported format FORMAT_RGB9E5_UFLOAT_PACK32");
	case gli::FORMAT_D16_UNORM_PACK16:
		return VK_FORMAT_D16_UNORM;
	case gli::FORMAT_D24_UNORM_PACK32:
		throw vcc_exception("Unsupported format FORMAT_D24_UNORM_PACK32");
	case gli::FORMAT_D32_SFLOAT_PACK32:
		return VK_FORMAT_D32_SFLOAT;
	case gli::FORMAT_S8_UINT_PACK8:
		return VK_FORMAT_S8_UINT;
	case gli::FORMAT_D16_UNORM_S8_UINT_PACK32:
		return VK_FORMAT_D16_UNORM_S8_UINT;
	case gli::FORMAT_D24_UNORM_S8_UINT_PACK32:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	case gli::FORMAT_RGB_DXT1_UNORM_BLOCK8:
		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case gli::FORMAT_RGB_DXT1_SRGB_BLOCK8:
		return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
	case gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT1_SRGB_BLOCK8:
		return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
	case gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT3_SRGB_BLOCK16:
		return VK_FORMAT_BC2_SRGB_BLOCK;
	case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT5_SRGB_BLOCK16:
		return VK_FORMAT_BC3_SRGB_BLOCK;
	case gli::FORMAT_R_ATI1N_UNORM_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_R_ATI1N_UNORM_BLOCK8");
	case gli::FORMAT_R_ATI1N_SNORM_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_R_ATI1N_UNORM_BLOCK8");
	case gli::FORMAT_RG_ATI2N_UNORM_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RG_ATI2N_UNORM_BLOCK16");
	case gli::FORMAT_RG_ATI2N_SNORM_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RG_ATI2N_SNORM_BLOCK16");
	case gli::FORMAT_RGB_BP_UFLOAT_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGB_BP_UFLOAT_BLOCK16");
	case gli::FORMAT_RGB_BP_SFLOAT_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGB_BP_SFLOAT_BLOCK16");
	case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGBA_BP_UNORM_BLOCK16");
	case gli::FORMAT_RGBA_BP_SRGB_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGBA_BP_SRGB_BLOCK16");
	case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8:
		return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		throw vcc_exception("Unsupported format FORMAT_RGB_ETC2_UNORM_BLOCK8");
	case gli::FORMAT_RGB_ETC2_SRGB_BLOCK8:
		return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ETC2_UNORM_BLOCK8:
		return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ETC2_SRGB_BLOCK8:
		return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ETC2_UNORM_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGBA_ETC2_UNORM_BLOCK16");
	case gli::FORMAT_RGBA_ETC2_SRGB_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGBA_ETC2_SRGB_BLOCK16");
	case gli::FORMAT_R_EAC_UNORM_BLOCK8:
		return VK_FORMAT_EAC_R11_UNORM_BLOCK;
	case gli::FORMAT_R_EAC_SNORM_BLOCK8:
		return VK_FORMAT_EAC_R11_SNORM_BLOCK;
	case gli::FORMAT_RG_EAC_UNORM_BLOCK16:
		return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
	case gli::FORMAT_RG_EAC_SNORM_BLOCK16:
		return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_4X4_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_5X4_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_5X4_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_5X5_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_5X5_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_6X5_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_6X5_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_6X6_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_6X6_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_8X5_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_8X5_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_8X6_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_8X6_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_8X8_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X5_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X5_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X6_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X6_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X8_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X8_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X10_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_10X10_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_12X10_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_12X10_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
	case gli::FORMAT_RGBA_ASTC_12X12_UNORM_BLOCK16:
		return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
	case gli::FORMAT_RGBA_ASTC_12X12_SRGB_BLOCK16:
		return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
	case gli::FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32:
		return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
	case gli::FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32:
		throw vcc_exception("Unsupported format FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32");
	case gli::FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32:
		throw vcc_exception("Unsupported format FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32");
	case gli::FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32:
		throw vcc_exception("Unsupported format FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32");
	case gli::FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32");
	case gli::FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32");
	case gli::FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32");
	case gli::FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32");
	case gli::FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8");
	case gli::FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8");
	case gli::FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8");
	case gli::FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8");
	case gli::FORMAT_RGB_ETC_UNORM_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_RGB_ETC_UNORM_BLOCK8");
	case gli::FORMAT_RGB_ATC_UNORM_BLOCK8:
		throw vcc_exception("Unsupported format FORMAT_RGB_ATC_UNORM_BLOCK8");
	case gli::FORMAT_RGBA_ATCA_UNORM_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGBA_ATCA_UNORM_BLOCK16");
	case gli::FORMAT_RGBA_ATCI_UNORM_BLOCK16:
		throw vcc_exception("Unsupported format FORMAT_RGBA_ATCI_UNORM_BLOCK16");
	case gli::FORMAT_L8_UNORM_PACK8:
		return VK_FORMAT_R8_UNORM;
	case gli::FORMAT_A8_UNORM_PACK8:
		return VK_FORMAT_R8_UNORM;
	case gli::FORMAT_LA8_UNORM_PACK8:
		return VK_FORMAT_R8G8_UNORM;
	case gli::FORMAT_L16_UNORM_PACK16:
		return VK_FORMAT_R16_UNORM;
	case gli::FORMAT_A16_UNORM_PACK16:
		return VK_FORMAT_R16_UNORM;
	case gli::FORMAT_LA16_UNORM_PACK16:
		return VK_FORMAT_R16G16_UNORM;
	case gli::FORMAT_BGR8_UNORM_PACK32:
		return VK_FORMAT_B8G8R8_UNORM;
	case gli::FORMAT_BGR8_SRGB_PACK32:
		return VK_FORMAT_B8G8R8_SRGB;
	case gli::FORMAT_RG3B2_UNORM_PACK8:
		throw vcc_exception("Unsupported format FORMAT_RG3B2_UNORM_PACK8");
	default:
		throw vcc_exception("Unknown format");
	}
}

}  // namespace internal
}  // namespace image
}  // namespace vcc
