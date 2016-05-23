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
#if !defined(VK_USE_PLATFORM_ANDROID_KHR) && defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif // VK_USE_PLATFORM_ANDROID_KHR

#include <vcc/surface.h>

namespace vcc {
namespace surface {

#ifdef VK_USE_PLATFORM_XCB_KHR

surface_type create(const type::supplier<<instance::instance_type> &instance, xcb_connection_t *connection, xcb_window_t window);

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef __ANDROID__

surface_type create(const type::supplier<instance::instance_type> &instance,
		ANativeWindow *window) {
	VkSurfaceKHR surface;
	VkAndroidSurfaceCreateInfoKHR create = {
		VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR, NULL, 0};
	create.window = window;
	VKCHECK(vkCreateAndroidSurfaceKHR(internal::get_instance(*instance), &create,
			NULL, &surface));
	return surface_type(surface, instance);
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

surface_type create(const type::supplier<instance::instance_type> &instance, HINSTANCE hinstance, HWND hwnd) {
	VkSurfaceKHR surface;
	VkWin32SurfaceCreateInfoKHR create = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, NULL, 0};
	create.hinstance = hinstance;
	create.hwnd = hwnd;
	VKCHECK(vkCreateWin32SurfaceKHR(internal::get_instance(*instance), &create, NULL, &surface));
	return surface_type(surface, instance);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

bool physical_device_support(VkPhysicalDevice device, surface_type &surface, uint32_t queueFamilyIndex) {
	VkBool32 supported;
	VKCHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, internal::get_instance(surface), &supported));
	return !!supported;
}

std::vector<VkSurfaceFormatKHR> physical_device_formats(VkPhysicalDevice device, surface_type &surface) {
	uint32_t count;
	VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, internal::get_instance(surface), &count, NULL));
	std::vector<VkSurfaceFormatKHR> formats(count);
	VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, internal::get_instance(surface), &count, formats.data()));
	return std::move(formats);
}

VkSurfaceCapabilitiesKHR physical_device_capabilities(VkPhysicalDevice device, surface_type &surface) {
	VkSurfaceCapabilitiesKHR capabilities;
	VKCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, internal::get_instance(surface), &capabilities));
	return capabilities;
}

std::vector<VkPresentModeKHR> physical_device_present_modes(VkPhysicalDevice device, surface_type &surface) {
	uint32_t count;
	VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, internal::get_instance(surface), &count, NULL));
	std::vector<VkPresentModeKHR> modes(count);
	VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, internal::get_instance(surface), &count, modes.data()));
	return std::move(modes);
}

}  // namespace surface
}  // namespace vcc
