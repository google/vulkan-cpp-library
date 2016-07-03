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
#if defined(__ANDROID__) || defined(ANDROID)

#include <android/bitmap.h>
#include <cassert>
#include <jni.h>
#include <sstream>
#include <vcc/image.h>
#include <vcc/internal/loader.h>
#include <vcc/memory.h>
#include <vcc/queue.h>

namespace vcc {
namespace image {

struct call_on_destroy {
	template<typename FunctorT>
	call_on_destroy(FunctorT functor) : functor(functor) {}
	~call_on_destroy() {
		functor();
	}

	std::function<void()> functor;
};

VkFormat android_format(AndroidBitmapFormat format) {
	switch (format) {
    case ANDROID_BITMAP_FORMAT_RGBA_8888:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case ANDROID_BITMAP_FORMAT_RGB_565:
	    return VK_FORMAT_R5G6B5_UNORM_PACK16;
    case ANDROID_BITMAP_FORMAT_RGBA_4444:
      return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    case ANDROID_BITMAP_FORMAT_A_8:
      return VK_FORMAT_R8_UNORM;
    default:
      throw std::runtime_error("unsupported android bitmap format");
  }
}

image::image_type create(
	const type::supplier<vcc::queue::queue_type> &queue, VkImageCreateFlags flags,
	VkImageUsageFlags usage, VkFormatFeatureFlags feature_flags,
	VkSharingMode sharingMode, const std::vector<uint32_t> &queueFamilyIndices,
	JNIEnv *env, jobject context, const char *resource_identifier) {

	const jclass context_class(env->GetObjectClass(context));
	assert(context_class);
	const jmethodID get_resources_method(env->GetMethodID(context_class,
		"getResources",
		"()Landroid/content/res/Resources;"));
	assert(get_resources_method);

	const jobject resources(env->CallObjectMethod(context, get_resources_method));
	assert(resources);

	const jclass resources_class(env->GetObjectClass(resources));
	assert(resources_class);
	const jmethodID get_identifier_method(env->GetMethodID(resources_class,
		"getIdentifier",
		"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I"));
	assert(get_resources_method);

	const jstring resource_identifier_string(env->NewStringUTF(
		resource_identifier));
	assert(resource_identifier_string);
	const jstring drawable_string(env->NewStringUTF("drawable"));
	assert(drawable_string);

	const jmethodID get_package_name_method(env->GetMethodID(context_class,
		"getPackageName", "()Ljava/lang/String;"));
	assert(get_package_name_method);
	const jstring package_name_string((jstring) env->CallObjectMethod(context,
		get_package_name_method));
	assert(package_name_string);

	const jint identifier(env->CallIntMethod(resources, get_identifier_method,
		resource_identifier_string, drawable_string, package_name_string));
	if (!identifier) {
		const char *package_name(env->GetStringUTFChars(package_name_string,
			nullptr));
		call_on_destroy release_string([env, package_name_string, package_name] () {
			env->ReleaseStringUTFChars(package_name_string, package_name);
		});

		std::stringstream ss;
		ss << "failed to find resource \"" << resource_identifier << "\""
			<< " in package \"" << package_name << "\"";
		std::string string(ss.str());
		VCC_PRINT("%s", string.c_str());
		throw std::runtime_error(std::move(string));
	}

	const jclass bitmap_factory_class(env->FindClass(
		"android/graphics/BitmapFactory"));
	assert(bitmap_factory_class);
	const jmethodID decode_resource_method(env->GetStaticMethodID(
		bitmap_factory_class, "decodeResource",
		"(Landroid/content/res/Resources;ILandroid/graphics/BitmapFactory$Options;)"
		"Landroid/graphics/Bitmap;"));
	assert(decode_resource_method);

	const jobject bitmap(env->CallStaticObjectMethod(bitmap_factory_class,
		decode_resource_method, resources, identifier, nullptr));
	assert(bitmap);

	AndroidBitmapInfo info;
	assert(AndroidBitmap_getInfo(env, bitmap, &info) == 0);

	void* addrPtr;
	assert(AndroidBitmap_lockPixels(env, bitmap, &addrPtr) == 0);
	call_on_destroy unlock_pixels([env, bitmap] () {
			assert(AndroidBitmap_unlockPixels(env, bitmap) == 0);
	});

	const VkFormat format(android_format(AndroidBitmapFormat(info.format)));
	const VkExtent3D extent{ info.width, info.height, 1 };
	const type::supplier<device::device_type> device(
		vcc::internal::get_parent(*queue));
	image::image_type image(image::create(device, 0, VK_IMAGE_TYPE_2D,
		format, extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR,
		usage, sharingMode, queueFamilyIndices, VK_IMAGE_LAYOUT_UNDEFINED));
	memory::bind(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, image);
	copy_to_linear_image(format, VK_IMAGE_ASPECT_COLOR_BIT,
		VkExtent2D{ extent.width, extent.height }, (uint8_t *) addrPtr,
		internal::bytes_per_pixel(format), info.stride, image);
	return std::move(image);
}

}  // namespace image
}  // namespace vcc

#endif  // __ANDROID__

