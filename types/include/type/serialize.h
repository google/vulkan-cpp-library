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
#ifndef TYPE_SERIALIZE_H_
#define TYPE_SERIALIZE_H_

#include <type/memory.h>
#include <type/view.h>

namespace type {

namespace internal {

const std::size_t base_alignment(16);

struct adapter {
	adapter(std::size_t element_size, std::size_t offset, std::size_t stride, std::size_t count)
		: element_size(element_size), offset(offset), stride(stride), count(count) {}

	virtual ~adapter() {}

	virtual bool dirty() const = 0;
	// Takes offset, stride for the layout into consideration.
	virtual void copy(void *destination) = 0;

	const std::size_t element_size, offset, stride, count;
};

template<typename T>
struct copy_type {
	typedef T value_type;

	static void copy(const value_type &value, void *destination) {
		value_type &target = *((value_type *)destination);
		target = value;
	}
};

template<typename T>
struct view_adapter : public adapter {

	view_adapter(memory_layout layout,
			const supplier<view_type<T>> &view,
			std::size_t offset, std::size_t stride)
		: adapter(calculate_element_size<T>(layout, view->size() > 1), offset, stride, view->size()),
		  view(view),
		  revision(REVISION_NONE) {}

	virtual bool dirty() const {
		return view->revision() > revision;
	}

	void copy(void *destination) {
		auto read(view->read());
		if (view->revision() > revision) {
			uint8_t *const d = (uint8_t *)destination + offset;
			for (std::size_t i = 0; i < count; ++i) {
				internal::copy_type<T>::copy(read[int(i)], &d[i * stride]);
			}
			revision = view->revision();
		}
	}

	const supplier<view_type<T>> view;
	revision_type revision;
};

template<typename T>
std::unique_ptr<adapter> make_adapter(memory_layout layout,
		const supplier<view_type<T>> &view,
		std::size_t offset, std::size_t stride) {
	return std::unique_ptr<adapter>(new view_adapter<T>(layout, view, offset, stride));
}

template<std::size_t N>
struct create_adapters_type {

	template<typename... T>
	static void create(memory_layout layout, const std::size_t *offsets, const std::size_t *strides,
			const std::tuple<supplier<view_type<T>>...>& views, std::vector<std::unique_ptr<adapter>> &adapters) {
		constexpr std::size_t index(N - 1);
		adapters[index] = make_adapter(layout, std::get<index>(views), offsets[index], strides[index]);
		create_adapters_type<index>::create(layout, offsets, strides, views, adapters);
	}
};
template<>
struct create_adapters_type<0> {

	template<typename... T>
	static void create(memory_layout layout, const std::size_t *offsets, const std::size_t *strides,
			const std::tuple<supplier<view_type<T>>...>& views, std::vector<std::unique_ptr<adapter>> &adapters) {}
};

template<typename... T>
static std::vector<std::unique_ptr<adapter>> create_adapters(
		memory_layout layout, const std::size_t *offsets,
		const std::size_t *strides, const std::tuple<supplier<view_type<T>>...>& views) {
	std::vector<std::unique_ptr<adapter>> adapters(sizeof...(T));
	create_adapters_type<sizeof...(T)>::create(layout, offsets, strides, views, adapters);
	return std::move(adapters);
}

}  // namespace internal

class serialize_type {
	friend std::size_t size(const serialize_type &serialize);
	friend void flush(const serialize_type &serialize, void *output);
	friend bool dirty(const serialize_type &serialize);
	friend memory_layout layout(const serialize_type &serialize);
private:
	typedef std::vector<std::unique_ptr<internal::adapter>> adapter_container_type;
	memory_layout layout;
	adapter_container_type adapters;
	std::size_t size;

	static std::size_t calculate_layout(memory_layout layout, std::size_t num_views,
			const std::size_t *sizes, const std::size_t *element_sizes,
			const std::size_t *base_alignments, std::size_t *offsets, std::size_t *strides);

public:

	serialize_type() = default;
	serialize_type(const serialize_type&) = delete;
	serialize_type(serialize_type&&) = default;
	serialize_type &operator=(const serialize_type&) = delete;
	serialize_type &operator=(serialize_type&&) = default;

	template<typename... T>
	serialize_type(memory_layout layout, const supplier<view_type<T>>&... views)
		: layout(layout) {
		constexpr std::size_t num_views(sizeof...(views));
		const std::size_t sizes[] = { views->size()... };
		const std::size_t element_sizes[] = { internal::calculate_element_size<T>(layout, views->is_array())... };
		const std::size_t base_alignments[] = { internal::calculate_base_alignment<T>(layout, views->is_array())... };
		std::size_t offsets[num_views], strides[num_views];
		size = calculate_layout(layout, num_views, sizes, element_sizes, base_alignments, offsets, strides);
		adapters = internal::create_adapters(layout, offsets, strides, std::make_tuple(views...));
	}
};

void flush(const serialize_type &serialize, void *output);
bool dirty(const serialize_type &serialize);
std::size_t size(const serialize_type &serialize);
memory_layout layout(const serialize_type &serialize);

template<typename... StorageType>
serialize_type make_serialize(memory_layout layout, StorageType... storages) {
	return serialize_type(layout, make_supplier(make_view(std::forward<StorageType>(storages)))...);
}

}  // namespace type

#endif // TYPE_SERIALIZE_H_
