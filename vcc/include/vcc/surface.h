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
#ifndef SURFACE_H_
#define SURFACE_H_

#ifdef __ANDROID__
#include <android/native_activity.h>
#endif  // __ANDROID__
#include <vcc/instance.h>

namespace vcc {
namespace surface {

struct surface_type
	: public internal::movable_destructible_with_parent<VkSurfaceKHR,
		instance::instance_type, vkDestroySurfaceKHR> {
#ifdef VK_USE_PLATFORM_XCB_KHR
		friend surface_type create(const type::supplier<instance::instance_type> &instance, xcb_connection_t *connection, xcb_window_t window);
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef __ANDROID__
		friend surface_type create(const type::supplier<instance::instance_type> &instance, ANativeWindow *window);
#endif // __ANDROID__
#ifdef VK_USE_PLATFORM_WIN32_KHR
		friend VCC_LIBRARY surface_type create(const type::supplier<instance::instance_type> &instance, HINSTANCE hinstance, HWND hwnd);
#endif // VK_USE_PLATFORM_WIN32_KHR

	surface_type() = default;
	surface_type(surface_type &&) = default;
	surface_type(const surface_type &) = delete;
	surface_type &operator=(surface_type &&) = default;
	surface_type &operator=(const surface_type &) = delete;

private:
	surface_type(VkSurfaceKHR instance,
		const type::supplier<instance::instance_type> &parent)
		: internal::movable_destructible_with_parent<VkSurfaceKHR,
		  instance::instance_type, vkDestroySurfaceKHR>(instance, parent) {}
};

#ifdef VK_USE_PLATFORM_XCB_KHR

surface_type create(const type::supplier<instance::instance_type> &instance, xcb_connection_t *connection, xcb_window_t window);

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef __ANDROID__

surface_type create(const type::supplier<instance::instance_type> &instance, ANativeWindow *window);

#endif // __ANDROID__

#ifdef VK_USE_PLATFORM_WIN32_KHR

VCC_LIBRARY surface_type create(const type::supplier<instance::instance_type> &instance, HINSTANCE hinstance, HWND hwnd);

#endif // VK_USE_PLATFORM_WIN32_KHR

VCC_LIBRARY bool physical_device_support(VkPhysicalDevice device, surface_type &surface, uint32_t queueFamilyIndex);

VCC_LIBRARY std::vector<VkSurfaceFormatKHR> physical_device_formats(VkPhysicalDevice device, surface_type &surface);

VCC_LIBRARY VkSurfaceCapabilitiesKHR physical_device_capabilities(VkPhysicalDevice device, surface_type &surface);
VCC_LIBRARY std::vector<VkPresentModeKHR> physical_device_present_modes(VkPhysicalDevice device, surface_type &surface);

}  // namespace surface
}  // namespace vcc

#endif /* SURFACE_H_ */
