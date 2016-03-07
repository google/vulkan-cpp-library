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
#ifndef INSTANCE_H_
#define INSTANCE_H_

#include <set>
#include <string>
#include <vcc/internal/raii.h>

namespace vcc {
namespace instance {

struct instance_type : public internal::movable_destructible<VkInstance, vkDestroyInstance> {
	friend VCC_LIBRARY instance_type create(
		const std::set<std::string> &layers,
		const std::set<std::string> &extensions);

	instance_type() = default;
	instance_type(instance_type &&) = default;
	instance_type(const instance_type &) = delete;
	instance_type &operator=(instance_type &&) = default;
	instance_type &operator=(const instance_type &) = delete;

private:
	explicit instance_type(VkInstance instance)
		: internal::movable_destructible<VkInstance, vkDestroyInstance>(instance) {}
};

VCC_LIBRARY instance_type create(
	const std::set<std::string> &layers = std::set<std::string>(),
	const std::set<std::string> &extensions = std::set<std::string>());

}  // namespace instance
}  // namespace vcc

#endif /* INSTANCE_H_ */
