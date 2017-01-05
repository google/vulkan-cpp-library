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
#include <vcc/sampler.h>

namespace vcc {
namespace sampler {

sampler_type create(const type::supplier<const device::device_type> &device,
		VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
		VkSamplerAddressMode addressModeW, float mipLodBias,
		VkBool32 anisotropyEnable, float maxAnisotropy, VkBool32 compareEnable,
		VkCompareOp compareOp, float minLod, float maxLod,
		VkBorderColor borderColor, VkBool32 unnormalizedCoordinates) {
	VkSamplerCreateInfo create = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, NULL, 0};
	create.magFilter = magFilter;
	create.minFilter = minFilter;
	create.mipmapMode = mipmapMode;
	create.addressModeU = addressModeU;
	create.addressModeV = addressModeV;
	create.addressModeW = addressModeW;
	create.mipLodBias = mipLodBias;
	//create.anisotropyEnable = anisotropyEnable;
	create.maxAnisotropy = maxAnisotropy;
	create.compareEnable = compareEnable;
	create.compareOp = compareOp;
	create.minLod = minLod;
	create.maxLod = maxLod;
	create.borderColor = borderColor;
	create.unnormalizedCoordinates = unnormalizedCoordinates;

	VkSampler sampler;
	VKCHECK(vkCreateSampler(internal::get_instance(*device), &create, NULL, &sampler));
	return sampler_type(sampler, device);
}

}  // namespace sampler
}  // namespace vcc
