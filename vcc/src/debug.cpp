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
#include <iostream>
#include <sstream>
#include <vcc/debug.h>
#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#endif // __ANDROID__

namespace vcc {
namespace debug {

const char *debug_severity(VkDebugReportFlagsEXT severity) {
	if (severity & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		return "ERROR";
	} else if (severity & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		return "WARNING";
	} else if (severity & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		return "PERFORMANCE WARNING";
	} else if (severity & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		return "INFO";
	} else if (severity & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		return "DEBUG";
	} else {
		return "MESSAGE";
	}
}

bool print_function(VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location,
		int32_t msgCode, const char* pLayerPrefix, const char* pMsg) {
#ifdef _WIN32
    std::stringstream ss;
    ss << debug_severity(flags) << ": [" << pLayerPrefix << "] Code "
        << msgCode << " : " << pMsg;
    const std::string string(ss.str());
    OutputDebugString(string.c_str());
#elif defined(__ANDROID__) || defined(ANDROID)
    __android_log_print(ANDROID_LOG_INFO, debug_severity(flags),
        "[%s] Code %d: %s", pLayerPrefix, msgCode, pMsg);
#else
    fprintf(stderr, debug_severity(flags), "[%s] Code %d: %s\n", pLayerPrefix,
        msgCode, pMsg);
#endif
    return false;
}

VkBool32 VKAPI_PTR dbgFunc(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData) {
	return (*static_cast<const callback_type *>(pUserData))(flags, objectType, object, location, messageCode, pLayerPrefix, pMessage) ? VK_TRUE : VK_FALSE;
}

debug_type create(const type::supplier<instance::instance_type> &instance,
		VkDebugReportFlagsEXT flags,
		std::unique_ptr<callback_type> &&callback_ptr,
		PFN_vkDebugReportCallbackEXT callback) {
	const PFN_vkCreateDebugReportCallbackEXT dbgCreateMsgCallback =
		(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
			internal::get_instance(*instance), "vkCreateDebugReportCallbackEXT");
	if (!dbgCreateMsgCallback) {
		throw vcc_exception("vkCreateDebugReportCallbackEXT");
	}
	assert(dbgCreateMsgCallback);
	const PFN_vkDestroyDebugReportCallbackEXT dbgDestroyMsgCallback =
		(PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
			internal::get_instance(*instance), "vkDestroyDebugReportCallbackEXT");
	if (!dbgDestroyMsgCallback) {
		throw vcc_exception("vkDestroyDebugReportCallbackEXT");
	}
	VkDebugReportCallbackEXT msg_callback;
	VkDebugReportCallbackCreateInfoEXT create = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT, NULL };
	create.flags = flags;
	create.pfnCallback = &dbgFunc;
	create.pUserData = callback_ptr.get();

	dbgCreateMsgCallback(internal::get_instance(*instance), &create, NULL, &msg_callback);
	return debug_type(instance, msg_callback,
		std::forward<std::unique_ptr<callback_type>>(callback_ptr),
		dbgDestroyMsgCallback);
}

debug_type create(type::supplier<instance::instance_type> &&instance, VkFlags msgFlags, const callback_type &callback) {
	return create(std::forward<type::supplier<instance::instance_type>>(instance), msgFlags, std::unique_ptr<callback_type>(new callback_type(callback)), &dbgFunc);
}

}  // namespace debug
}  // namespace vcc
