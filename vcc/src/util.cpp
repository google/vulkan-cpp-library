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
#include <algorithm>
#include <cstdarg>
#include <functional>
#include <iterator>
#include <sstream>
#include <vcc/util.h>

#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#endif  // __ANDROID__

namespace vcc {

const char *vkresult_string(VkResult result) {
	switch (result) {
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	/*case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";*/
	case VK_RESULT_RANGE_SIZE:
		return "VK_RESULT_RANGE_SIZE";
	case VK_RESULT_MAX_ENUM:
		return "VK_RESULT_MAX_ENUM";
	default:
		return "<unknown>";
	}
}

void vcc_exception::maybe_throw(VkResult result, const char *file, int line,
		const char *function, const char *expr) {
	if (result != VK_SUCCESS) {
		std::stringstream ss;
		ss << file << "(" << line << "): In " << function << ": Expression "
			<< expr << " resulted in " << vkresult_string(result);
		std::string string(ss.str());
		util::diagnostic_print(file, function, line, "%s", string.c_str());
		throw vcc_exception(std::move(string));
	}
}

namespace util {

std::vector<const char *> to_pointers(const std::vector<std::string> &vector) {
	std::vector<const char *> pointer_vector;
	pointer_vector.reserve(vector.size());
	std::transform(vector.begin(), vector.end(), std::back_inserter(pointer_vector),
			std::bind(&std::string::c_str, std::placeholders::_1));
	return std::move(pointer_vector);
}

std::vector<const char *> to_pointers(const std::set<std::string> &set) {
	std::vector<const char *> pointer_vector;
	pointer_vector.reserve(set.size());
	std::transform(set.begin(), set.end(), std::back_inserter(pointer_vector),
		std::bind(&std::string::c_str, std::placeholders::_1));
	return std::move(pointer_vector);
}

void diagnostic_print(const char *filename, const char *function, int line, const char *fmt, ...) {
#if 1
	va_list args;
	va_start(args, fmt);
#if defined(__ANDROID__) || defined(ANDROID)
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s:%d", filename, line);
	// Android logger does not support very long strings (~4000 characters).
	const size_t size(vsnprintf(nullptr, 0, fmt, args));
	std::string output(size, '\0');
	vsprintf(&output[0], fmt, args);
	std::stringstream ss(std::move(output));
	while (ss) {
		std::string line;
		std::getline(ss, line);
		__android_log_write(ANDROID_LOG_INFO, buffer, line.c_str());
	}
#else
	fprintf(stderr, "%s(In %s:%d): ", filename, function, line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif  // __ANDROID__
	va_end(args);
#else
	std::stringstream ss;
	ss << filename << "(In " << function << ':' << line << "): ";
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);
	ss << buffer << std::endl;
	OutputDebugString(ss.str().c_str());
#endif
}

}  // namespace util
}  // namespace vcc
