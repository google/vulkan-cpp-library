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
#include <vcc/descriptor_set.h>
#include <vcc/descriptor_set_layout.h>

namespace vcc {
namespace descriptor_set {

std::vector<descriptor_set_type> create(
	const type::supplier<device::device_type> &device,
	const type::supplier<descriptor_pool::descriptor_pool_type> &descriptor_pool,
	const std::vector<type::supplier<
		descriptor_set_layout::descriptor_set_layout_type>> &setLayouts) {
	VkDescriptorSetAllocateInfo allocate = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL };
	allocate.descriptorPool = vcc::internal::get_instance(*descriptor_pool);
	allocate.descriptorSetCount = (uint32_t) setLayouts.size();
	std::vector<VkDescriptorSetLayout> converted_set_layouts;
	converted_set_layouts.reserve(setLayouts.size());
	std::transform(setLayouts.begin(), setLayouts.end(),
		std::back_inserter(converted_set_layouts),
		[](const type::supplier<descriptor_set_layout::descriptor_set_layout_type>
			&set_layout) {
		return vcc::internal::get_instance(*set_layout);
	});
	allocate.pSetLayouts = converted_set_layouts.data();
	std::vector<VkDescriptorSet> descriptor_sets(setLayouts.size());
	VKCHECK(vkAllocateDescriptorSets(vcc::internal::get_instance(*device), &allocate,
		&descriptor_sets[0]));
	std::vector<descriptor_set_type> converted_descriptor_sets;
	converted_descriptor_sets.reserve(descriptor_sets.size());
	std::transform(descriptor_sets.begin(), descriptor_sets.end(),
		std::back_inserter(converted_descriptor_sets),
		[&device, &descriptor_pool](VkDescriptorSet descriptor_set) {
		return descriptor_set_type(descriptor_set, descriptor_pool, device);
	});
	return std::move(converted_descriptor_sets);
}

namespace internal {

void add(update_storage &storage, const copy &c) {
	VkCopyDescriptorSet set = {VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET, NULL};
	set.srcSet = vcc::internal::get_instance(*c.src_set);
	set.srcBinding = c.src_binding;
	set.srcArrayElement = c.src_array_element;
	set.dstSet = vcc::internal::get_instance(*c.dst_set);
	set.dstBinding = c.dst_binding;
	set.dstArrayElement = c.dst_array_element;
	set.descriptorCount = c.descriptor_count;
	storage.copy_sets.push_back(set);
	for (uint32_t i = 0; i < c.descriptor_count; ++i) {
		c.dst_set->references.clone(vcc::internal::bind_point_type{ c.dst_binding, uint32_t(c.dst_array_element + i) }, c.src_set->references);
	}
}

void add(update_storage &storage, const write_image &write) {
	VkWriteDescriptorSet set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL};
	set.dstSet = vcc::internal::get_instance(*write.dst_set);
	set.dstBinding = write.dst_binding;
	set.dstArrayElement = write.dst_array_element;
	set.descriptorCount = (uint32_t) write.images.size();
	set.descriptorType = write.descriptor_type;
	std::vector<VkDescriptorImageInfo> image_infos;
	image_infos.reserve(write.images.size());
	for (const image_info &info : write.images) {
		image_infos.push_back(VkDescriptorImageInfo{
			vcc::internal::get_instance(*info.sampler),
			vcc::internal::get_instance(*info.image_view), info.image_layout});
	}
	set.pImageInfo = image_infos.data();
	storage.image_infos.push_back(std::move(image_infos));
	storage.write_sets.push_back(set);
	for (uint32_t i = 0; i < write.images.size(); ++i) {
		write.dst_set->references.put(vcc::internal::bind_point_type{
			write.dst_binding, uint32_t(write.dst_array_element + i) },
			write.images[i].sampler, write.images[i].image_view);
	}
}

void add(update_storage &storage, const write_buffer_type &write) {
	VkWriteDescriptorSet set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL};
	set.dstSet = vcc::internal::get_instance(*write.dst_set);
	set.dstBinding = write.dst_binding;
	set.dstArrayElement = write.dst_array_element;
	set.descriptorCount = (uint32_t) write.buffers.size();
	set.descriptorType = write.descriptor_type;
	std::vector<VkDescriptorBufferInfo> buffer_infos;
	buffer_infos.reserve(write.buffers.size());
	for (const buffer_info_type &info : write.buffers) {
		buffer_infos.push_back(VkDescriptorBufferInfo{ vcc::internal::get_instance(*info.buffer), info.offset, info.range});
	}
	set.pBufferInfo = buffer_infos.data();
	storage.buffer_infos.push_back(std::move(buffer_infos));
	storage.write_sets.push_back(set);
	for (uint32_t i = 0; i < write.buffers.size(); ++i) {
		const vcc::internal::bind_point_type bind_point{ write.dst_binding, uint32_t(write.dst_array_element + i) };
		write.dst_set->references.put(bind_point, write.buffers[i].buffer);
	}
}

void add(update_storage &storage, const write_buffer_view_type &write) {
	VkWriteDescriptorSet set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL};
	set.dstSet = vcc::internal::get_instance(*write.dst_set);
	set.dstBinding = write.dst_binding;
	set.dstArrayElement = write.dst_array_element;
	set.descriptorCount = (uint32_t) write.buffers.size();
	set.descriptorType = write.descriptor_type;
	std::vector<VkBufferView> buffer_views;
	buffer_views.reserve(write.buffers.size());
	for (const type::supplier<buffer_view::buffer_view_type> &view : write.buffers) {
		buffer_views.push_back(vcc::internal::get_instance(*view));
	}
	set.pTexelBufferView = buffer_views.data();
	storage.buffer_view.push_back(std::move(buffer_views));
	storage.write_sets.push_back(set);
	for (uint32_t i = 0; i < write.buffers.size(); ++i) {
		write.dst_set->references.put(vcc::internal::bind_point_type{ write.dst_binding, uint32_t(write.dst_array_element + i) },
			write.buffers[i]);
	}
}

void count(update_storage &storage, const write_image &write) {
	++storage.write_sets_size;
	storage.image_info_size += write.images.size();
}

void count(update_storage &storage, const write_buffer_type &write) {
	++storage.write_sets_size;
	storage.buffer_info_size += write.buffers.size();
}

void count(update_storage &storage, const write_buffer_view_type &write) {
	++storage.write_sets_size;
	storage.buffer_view_size += write.buffers.size();
}

void update_storage::reserve() {
	copy_sets.reserve(copy_sets_size);
	write_sets.reserve(write_sets_size);
	image_infos.reserve(image_info_size);
	buffer_infos.reserve(buffer_info_size);
	buffer_view.reserve(buffer_view_size);
}

}  // namespace internal

}  // namespace descriptor_set
}  // namespace vcc
