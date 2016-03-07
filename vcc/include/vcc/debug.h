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
#ifndef _VCC_DEBUG_H_
#define _VCC_DEBUG_H_

#include <vcc/instance.h>
#include <vulkan/vk_lunarg_debug_marker.h>

namespace vcc {
namespace debug {

typedef std::function<bool(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT,
	uint64_t, size_t, int32_t, const char*, const char*)> callback_type;

struct debug_type {
	friend VCC_LIBRARY debug_type create(
		const type::supplier<instance::instance_type> &instance,
		VkDebugReportFlagsEXT flags,
		std::unique_ptr<callback_type> &&callback_ptr,
		PFN_vkDebugReportCallbackEXT callback);

	debug_type()
		: msg_callback(VK_NULL_HANDLE), dbgDestroyMsgCallback(nullptr) {}
	debug_type(debug_type &&copy) : instance(std::move(copy.instance)),
		msg_callback(copy.msg_callback), callback(std::move(copy.callback)),
		dbgDestroyMsgCallback(copy.dbgDestroyMsgCallback) {
		msg_callback = VK_NULL_HANDLE;
		dbgDestroyMsgCallback = nullptr;
	}
	debug_type &operator=(const debug_type &) = delete;
	debug_type &operator=(debug_type &&copy) {
		instance = std::move(copy.instance);
		msg_callback = copy.msg_callback;
		callback = std::move(copy.callback);
		dbgDestroyMsgCallback = copy.dbgDestroyMsgCallback;
		copy.msg_callback = VK_NULL_HANDLE;
		copy.dbgDestroyMsgCallback = nullptr;
		return *this;
	}

	type::supplier<instance::instance_type> instance;
	VkDebugReportCallbackEXT msg_callback;
	std::unique_ptr<callback_type> callback;
	PFN_vkDestroyDebugReportCallbackEXT dbgDestroyMsgCallback;

	~debug_type() {
		if (msg_callback && dbgDestroyMsgCallback) {
			dbgDestroyMsgCallback(internal::get_instance(*instance),
				msg_callback, NULL);
		}
	}

private:
	debug_type(const type::supplier<instance::instance_type> &instance,
		VkDebugReportCallbackEXT msg_callback,
		std::unique_ptr<callback_type> &&callback,
		PFN_vkDestroyDebugReportCallbackEXT dbgDestroyMsgCallback)
		: instance(instance), msg_callback(msg_callback),
		  callback(std::forward<std::unique_ptr<callback_type>>(callback)),
		  dbgDestroyMsgCallback(dbgDestroyMsgCallback) {}
};

// Can be used as callback, prints using OutputDebugString on Windows and std::cerr on non-windows systems.
// Does not stop execution.
VCC_LIBRARY bool print_function(VkDebugReportFlagsEXT,
	VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*,
	const char*);

VCC_LIBRARY debug_type create(
	type::supplier<instance::instance_type> &&instance, VkFlags msgFlags,
	const callback_type &callback = &print_function);

}  // namespace debug
}  // namespace vcc

#endif  // _VCC_DEBUG_H_
