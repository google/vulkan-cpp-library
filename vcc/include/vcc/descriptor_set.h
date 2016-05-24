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
#ifndef DESCRIPTOR_SET_H_
#define DESCRIPTOR_SET_H_

#include <algorithm>
#include <vcc/buffer.h>
#include <vcc/buffer_view.h>
#include <vcc/device.h>
#include <vcc/descriptor_pool.h>
#include <vcc/descriptor_set_layout.h>
#include <vcc/image_view.h>
#include <vcc/input_buffer.h>
#include <vcc/internal/hook.h>
#include <vcc/util.h>

namespace vcc {

namespace queue {
struct queue_type;
}  // namespace queue

namespace descriptor_set {

struct descriptor_set_type
	: internal::movable_allocated_with_pool_parent2<VkDescriptorSet,
	  device::device_type, descriptor_pool::descriptor_pool_type,
	  vkFreeDescriptorSets> {
	descriptor_set_type() = default;
	descriptor_set_type(const descriptor_set_type &) = delete;
	descriptor_set_type(descriptor_set_type &&copy)
		: internal::movable_allocated_with_pool_parent2<VkDescriptorSet,
			device::device_type, descriptor_pool::descriptor_pool_type,
			vkFreeDescriptorSets>(std::forward<descriptor_set_type>(copy)),
		  pre_execute_callbacks(std::move(copy.pre_execute_callbacks)) {}
	descriptor_set_type &operator=(const descriptor_set_type&) = delete;
	descriptor_set_type(VkDescriptorSet instance,
		const type::supplier<descriptor_pool::descriptor_pool_type> &pool,
		const type::supplier<device::device_type> &parent)
		: internal::movable_allocated_with_pool_parent2<VkDescriptorSet,
			device::device_type, descriptor_pool::descriptor_pool_type,
			vkFreeDescriptorSets>(instance, pool, parent) {}

	internal::hook_map_type<std::pair<uint32_t, uint32_t>,
		util::hash_pair<uint32_t, uint32_t>, queue::queue_type &> pre_execute_callbacks;
	internal::reference_map_type<std::pair<uint32_t, uint32_t>,
		util::hash_pair<uint32_t, uint32_t>> references;

};

VCC_LIBRARY std::vector<descriptor_set_type> create(
	const type::supplier<device::device_type> &device,
	const type::supplier<vcc::descriptor_pool::descriptor_pool_type>
		&descriptor_pool,
	const std::vector<type::supplier<
		vcc::descriptor_set_layout::descriptor_set_layout_type>> &setLayouts);

struct copy {
	type::supplier<descriptor_set_type> src_set;
	uint32_t src_binding, src_array_element;
	type::supplier<descriptor_set_type> dst_set;
	uint32_t dst_binding, dst_array_element, descriptor_count;
};

struct image_info {
	type::supplier<sampler::sampler_type> sampler;
	type::supplier<image_view::image_view_type> image_view;
	VkImageLayout image_layout;
};

struct buffer_info_type {
	type::supplier<buffer::buffer_type> buffer;
	VkDeviceSize offset, range;
};

inline buffer_info_type buffer_info(
		const type::supplier<buffer::buffer_type> &buffer,
		VkDeviceSize offset, VkDeviceSize range) {
	return buffer_info_type{buffer, offset, range};
}

struct buffer_info_data_type {
	type::supplier<input_buffer::input_buffer_type> buffer;
	VkDeviceSize offset, range;
};

VCC_LIBRARY buffer_info_data_type buffer_info(
	const type::supplier<input_buffer::input_buffer_type> &buffer,
	VkDeviceSize offset, VkDeviceSize range);
VCC_LIBRARY buffer_info_data_type buffer_info(
	const type::supplier<input_buffer::input_buffer_type> &buffer);

struct write_image {
	type::supplier<descriptor_set_type> dst_set;
	uint32_t dst_binding, dst_array_element;
	// Must be any of the following only
	// VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_
	// IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_
	// STORAGE_IMAGE or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
	VkDescriptorType descriptor_type;
	std::vector<image_info> images;
};

struct write_buffer_type {
	type::supplier<descriptor_set_type> dst_set;
	uint32_t dst_binding, dst_array_element;
	// Must be any of the following only
	// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_
	// STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_
	// TYPE_STORAGE_BUFFER_DYNAMIC
	VkDescriptorType descriptor_type;
	std::vector<buffer_info_type> buffers;
};

inline write_buffer_type write_buffer(
		const type::supplier<descriptor_set_type> &dst_set,
		uint32_t dst_binding, uint32_t dst_array_element,
		VkDescriptorType descriptor_type,
		const std::vector<buffer_info_type> &buffers) {
	return write_buffer_type{dst_set,
		dst_binding, dst_array_element, descriptor_type, buffers};
}

struct write_buffer_data_type {
	type::supplier<descriptor_set_type> dst_set;
	uint32_t dst_binding, dst_array_element;
	// Must be any of the following
	// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_
	// STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_
	// TYPE_STORAGE_BUFFER_DYNAMIC
	VkDescriptorType descriptor_type;
	std::vector<buffer_info_data_type> buffers;
};

inline write_buffer_data_type write_buffer(
	const type::supplier<descriptor_set_type> &dst_set,
	uint32_t dst_binding, uint32_t dst_array_element,
	VkDescriptorType descriptor_type,
	const std::vector<buffer_info_data_type> &buffers) {
	return write_buffer_data_type{ dst_set, dst_binding,
		dst_array_element, descriptor_type, buffers };
}

struct write_buffer_view_type {
	type::supplier<descriptor_set_type> dst_set;
	uint32_t dst_binding, dst_array_element;
	// Must be any of the following only
	// VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER or VK_DESCRIPTOR_
	// TYPE_STORAGE_TEXEL_BUFFER
	VkDescriptorType descriptor_type;
	std::vector<type::supplier<buffer_view::buffer_view_type>> buffers;
};

inline write_buffer_view_type write_buffer_view(
		type::supplier<descriptor_set_type> &&dst_set,
		uint32_t dst_binding, uint32_t dst_array_element,
		VkDescriptorType descriptor_type,
		const std::vector<type::supplier<buffer_view::buffer_view_type>> &buffers) {
	return write_buffer_view_type{
		std::forward<type::supplier<descriptor_set_type>>(dst_set),
		dst_binding, dst_array_element, descriptor_type, buffers };
}

namespace internal {

struct update_storage {
	std::vector<VkCopyDescriptorSet> copy_sets;
	std::vector<VkWriteDescriptorSet> write_sets;
	std::vector<std::vector<VkDescriptorImageInfo>> image_infos;
	std::vector<std::vector<VkDescriptorBufferInfo>> buffer_infos;
	std::vector<std::vector<VkBufferView>> buffer_view;

	VCC_LIBRARY void reserve();

	std::size_t copy_sets_size = 0, write_sets_size = 0, image_info_size = 0,
		buffer_info_size = 0, buffer_view_size = 0;
};

VCC_LIBRARY void add(update_storage &storage, const copy &);
VCC_LIBRARY void add(update_storage &storage, const write_image &);
VCC_LIBRARY void add(update_storage &storage, const write_buffer_type &);
VCC_LIBRARY void add(update_storage &storage, const write_buffer_view_type &);
VCC_LIBRARY void add(update_storage &storage, const write_buffer_data_type &);

VCC_LIBRARY void count(update_storage &storage, const write_image &);
VCC_LIBRARY void count(update_storage &storage, const write_buffer_type &);
VCC_LIBRARY void count(update_storage &storage, const write_buffer_view_type &);
VCC_LIBRARY void count(update_storage &storage, const write_buffer_data_type &);

}  // namespace internal

// takes device plus a list of copy and write_* items.
template <typename... ArgsT>
void update(device::device_type &device, const ArgsT &... args) {
	internal::update_storage storage;
	util::internal::pass((internal::count(storage, args), 1)...);
	storage.reserve();
	util::internal::pass((internal::add(storage, args), 1)...);
	const std::set<std::mutex *> mutexes(
		util::set_from_variadic_movables<std::mutex *>(
			&vcc::internal::get_mutex(*args.dst_set)...));
	std::vector<std::unique_lock<std::mutex>> deferred_locks;
	deferred_locks.reserve(mutexes.size());
	std::transform(mutexes.begin(), mutexes.end(),
		std::back_inserter<decltype(deferred_locks)>(deferred_locks),
		[](std::mutex *mutex) {
		return std::unique_lock<std::mutex>(*mutex, std::defer_lock);
	});
	vcc::util::lock(deferred_locks);
	VKTRACE(vkUpdateDescriptorSets(vcc::internal::get_instance(device),
		(uint32_t)storage.write_sets.size(), storage.write_sets.data(),
		(uint32_t)storage.copy_sets.size(), storage.copy_sets.data()));
}

}  // namespace descriptor_set
}  // namespace vcc

#endif /* DESCRIPTOR_SET_H_ */
