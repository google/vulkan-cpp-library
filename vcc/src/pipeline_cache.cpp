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
#include <iterator>
#include <vcc/pipeline_cache.h>

namespace vcc {
namespace pipeline_cache {

pipeline_cache_type create(const type::supplier<device::device_type> &device, const std::string &data) {
	VkPipelineCacheCreateInfo create = {VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, NULL, 0};
	// TODO(gardell): Support
	create.initialDataSize = data.size();
	create.pInitialData = &data[0];
	VkPipelineCache cache;
	VKCHECK(vkCreatePipelineCache(internal::get_instance(*device), &create, NULL, &cache));
	return pipeline_cache_type(cache, device);
}

std::string serialize(const pipeline_cache_type &pipeline_cache) {
	size_t data_size;
	VKCHECK(vkGetPipelineCacheData(
		internal::get_instance(*internal::get_parent(pipeline_cache)),
		internal::get_instance(pipeline_cache), &data_size, NULL));
	std::string string(data_size, 0);
	VKCHECK(vkGetPipelineCacheData(
		internal::get_instance(*internal::get_parent(pipeline_cache)),
		internal::get_instance(pipeline_cache), &data_size, &string[0]));
	return std::move(string);
}

void merge(const pipeline_cache_type &pipeline_cache,
		const std::vector<type::supplier<pipeline_cache_type>> &source_caches) {
	std::vector<VkPipelineCache> caches;
	caches.reserve(source_caches.size());
	std::transform(source_caches.begin(), source_caches.end(), std::back_inserter(caches),
			[](const type::supplier<pipeline_cache_type> &cache){
		return internal::get_instance(*cache);
	});
	VKCHECK(vkMergePipelineCaches(internal::get_instance(*internal::get_parent(pipeline_cache)),
		internal::get_instance(pipeline_cache), (uint32_t) caches.size(), &caches.front()));
}

}  // namespace pipeline_cache
}  // namespace vcc
