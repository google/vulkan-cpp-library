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
#ifndef SAMPLER_H_
#define SAMPLER_H_

#include <vcc/device.h>

namespace vcc {
namespace sampler {

struct sampler_type : internal::movable_destructible_with_parent<VkSampler,
		const device::device_type, vkDestroySampler> {

	friend VCC_LIBRARY sampler_type create(
		const type::supplier<const device::device_type> &, VkFilter, VkFilter, VkSamplerMipmapMode,
		VkSamplerAddressMode, VkSamplerAddressMode, VkSamplerAddressMode, float, VkBool32, float,
		VkBool32, VkCompareOp, float, float, VkBorderColor, VkBool32);

	sampler_type() = default;
	sampler_type(sampler_type &&) = default;
	sampler_type(const sampler_type &) = delete;
	sampler_type &operator=(sampler_type &&) = default;
	sampler_type &operator=(const sampler_type &) = delete;

private:
	sampler_type(VkSampler instance, const type::supplier<const device::device_type> &parent)
		: movable_destructible_with_parent(instance, parent) {}
};

VCC_LIBRARY sampler_type create(const type::supplier<const device::device_type> &device,
	VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
	VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
	VkSamplerAddressMode addressModeW, float mipLodBias, VkBool32 anisotropyEnable,
	float maxAnisotropy, VkBool32 compareEnable, VkCompareOp compareOp, float minLod, float maxLod,
	VkBorderColor borderColor, VkBool32 unnormalizedCoordinates);

}  // namespace sampler
}  // namespace vcc

#endif /* SAMPLER_H_ */
