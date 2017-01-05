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
#ifndef PIPELINE_CACHE_H_
#define PIPELINE_CACHE_H_

#include <vcc/device.h>

namespace vcc {
namespace pipeline_cache {

struct pipeline_cache_type : internal::movable_destructible_with_parent<VkPipelineCache,
		const device::device_type, vkDestroyPipelineCache> {

	friend VCC_LIBRARY pipeline_cache_type create(
		const type::supplier<const device::device_type> &, const std::string &);

	pipeline_cache_type() = default;
	pipeline_cache_type(pipeline_cache_type &&) = default;
	pipeline_cache_type(const pipeline_cache_type &) = delete;
	pipeline_cache_type &operator=(pipeline_cache_type &&) = default;
	pipeline_cache_type &operator=(const pipeline_cache_type &) = delete;

private:
	pipeline_cache_type(VkPipelineCache instance,
		const type::supplier<const device::device_type> &parent)
		: movable_destructible_with_parent(instance, parent) {}
};

VCC_LIBRARY pipeline_cache_type create(const type::supplier<const device::device_type> &device,
	const std::string &data = std::string());

VCC_LIBRARY std::string serialize(const pipeline_cache_type &pipeline_cache);

VCC_LIBRARY void merge(const pipeline_cache_type &pipeline_cache,
	const std::vector<type::supplier<const pipeline_cache_type>> &source_caches);

}  // namespace pipeline_cache
}  // namespace vcc


#endif /* PIPELINE_CACHE_H_ */
