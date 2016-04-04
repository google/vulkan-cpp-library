/*
 * Copyright 2016 Google Inc. All Rights Reserved.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAII_H_
#define RAII_H_

#include <functional>
#include <mutex>
#include <type/supplier.h>
#include <vcc/util.h>

namespace vcc {
namespace internal {

template<typename T>
struct handle_type {

	handle_type() : handle(VK_NULL_HANDLE) {}
	handle_type(T handle) : handle(handle) {}
	handle_type(const handle_type<T> &) = default;
	handle_type(handle_type<T> &&copy) : handle(copy.handle) {
		copy.handle = VK_NULL_HANDLE;
	}
	handle_type &operator=(const handle_type<T> &) = default;
	handle_type &operator=(handle_type<T> &&copy) {
		handle = copy.handle;
		copy.handle = VK_NULL_HANDLE;
		return *this;
	}

	T handle;
	operator T &() {
		return handle;
	}
	operator const T &() const {
		return handle;
	}
	T *operator&() {
		return &handle;
	}
	const T *operator&() const {
		return &handle;
	}
};

template<typename T>
auto get_instance(const T &value)->const decltype(value.instance)& {
	return value.instance;
}

template<typename T>
auto get_mutex(const T &value)->decltype(value.mutex)& {
	return value.mutex;
}

template<typename T>
auto get_parent(T &value)->decltype(value.parent)& {
	return value.parent;
}

template<typename T>
auto get_parent(const T &value)->const decltype(value.parent)& {
	return value.parent;
}

template<typename T>
auto get_memory(T &value)->decltype(value.memory)& {
	return value.memory;
}

template<typename T>
VkDeviceSize &get_offset(T &value) {
	return value.offset;
}

// This generic is used for VkQueue.
template<typename T, typename ParentT>
struct movable_with_parent {
	template<typename U>
	friend auto get_instance(const U &value)->const decltype(value.instance)&;
	template<typename U>
	friend auto get_mutex(const U &value)->decltype(value.mutex)&;
	template<typename U>
	friend auto get_parent(U &value)->decltype(value.parent)&;
	template<typename U>
	friend auto get_parent(const U &value)->const decltype(value.parent)&;

protected:
	movable_with_parent() = default;
	movable_with_parent(const movable_with_parent &) = delete;
	movable_with_parent(movable_with_parent &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
	}
	movable_with_parent &operator=(const movable_with_parent &) = delete;
	movable_with_parent &operator=(movable_with_parent &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
		return *this;
	}

	movable_with_parent(T instance, const type::supplier<ParentT> &parent)
		: instance(instance), parent(parent) {}

private:
	handle_type<T> instance;
	type::supplier<ParentT> parent;
	mutable std::mutex mutex;
};

// This generic is used for VkInstance and VkDevice.
template<typename T,
	void (VKAPI_PTR *PFN_vkDestroy)(T , const VkAllocationCallbacks*)>
struct movable_destructible {
	template<typename U>
	friend auto get_instance(const U &value)->const decltype(value.instance)&;
	template<typename U>
	friend auto get_mutex(const U &value)->decltype(value.mutex)&;
	typedef T value_type;

protected:
	movable_destructible() : instance(VK_NULL_HANDLE) {};
	movable_destructible(const movable_destructible &) = delete;
	movable_destructible(movable_destructible &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
	}
	movable_destructible &operator=(const movable_destructible &) = delete;
	movable_destructible &operator=(movable_destructible &&copy) {
		destroy();
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		return *this;
	}

	~movable_destructible() {
		destroy();
	}

	explicit movable_destructible(T instance) : instance(instance) {}
private:
	handle_type<T> instance;
	mutable std::mutex mutex;

	void destroy() {
		if (instance) {
			std::lock_guard<std::mutex> lock(mutex);
			PFN_vkDestroy(instance, NULL);
		}
	}
};

// This generic is used for types that require a VkDevice when destroyed.
template<typename T, typename ParentT, void (VKAPI_PTR *PFN_vkDestroy)(
		typename ParentT::value_type, T, const VkAllocationCallbacks*)>
struct movable_destructible_with_parent {
	template<typename U>
	friend auto get_instance(const U &value)->const decltype(value.instance)&;
	template<typename U>
	friend auto get_mutex(const U &value)->decltype(value.mutex)&;
	template<typename U>
	friend auto get_parent(U &value)->decltype(value.parent)&;
	template<typename U>
	friend auto get_parent(const U &value)->const decltype(value.parent)&;
	typedef T value_type;

	operator bool() const {
		return instance && parent;
	}

protected:
	movable_destructible_with_parent() = default;
	movable_destructible_with_parent(
		const movable_destructible_with_parent &) = delete;
	movable_destructible_with_parent(
			movable_destructible_with_parent &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
	}
	movable_destructible_with_parent &operator=(
		const movable_destructible_with_parent &) = delete;
	movable_destructible_with_parent &operator=(
			movable_destructible_with_parent &&copy) {
		destroy();
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
		return *this;
	}

	~movable_destructible_with_parent() {
		destroy();
	}

	movable_destructible_with_parent(T instance,
		const type::supplier<ParentT> &parent)
		: instance(instance),
		  parent(parent) {}

private:
	handle_type<T> instance;
	type::supplier<ParentT> parent;
	mutable std::mutex mutex;

	void destroy() {
		if (instance && parent) {
			std::lock_guard<std::mutex> lock(mutex);
			PFN_vkDestroy(get_instance(*parent), instance, NULL);
		}
	}
};

// This generic is used for allocated types like command buffers.
template<typename T, typename ParentT, typename PoolT,
	VKAPI_ATTR void (VKAPI_PTR *PFN_vkFree)(typename ParentT::value_type,
		typename PoolT::value_type, uint32_t, const T*)>
struct movable_allocated_with_pool_parent1 {
	template<typename U>
	friend auto get_instance(const U &value)->const decltype(value.instance)&;
	template<typename U>
	friend auto get_mutex(const U &value)->decltype(value.mutex)&;
	template<typename U>
	friend auto get_parent(U &value)->decltype(value.parent)&;
	template<typename U>
	friend auto get_parent(const U &value)->const decltype(value.parent)&;
	typedef T value_type;

protected:
	movable_allocated_with_pool_parent1() = default;
	movable_allocated_with_pool_parent1(
		const movable_allocated_with_pool_parent1 &) = delete;
	movable_allocated_with_pool_parent1(
			movable_allocated_with_pool_parent1 &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		pool = std::move(copy.pool);
		parent = std::move(copy.parent);
	}
	movable_allocated_with_pool_parent1 &operator=(
		const movable_allocated_with_pool_parent1 &) = delete;
	movable_allocated_with_pool_parent1 &operator=(
			movable_allocated_with_pool_parent1 &&copy) {
		destroy();
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		pool = std::move(copy.pool);
		parent = std::move(copy.parent);
		return *this;
	}

	~movable_allocated_with_pool_parent1() {
		destroy();
	}

	movable_allocated_with_pool_parent1(T instance,
		const type::supplier<PoolT> &pool,
		const type::supplier<ParentT> &parent)
		: instance(instance), pool(pool), parent(parent) {}

private:
	handle_type<T> instance;
	type::supplier<PoolT> pool;
	type::supplier<ParentT> parent;
	mutable std::mutex mutex;

	void destroy() {
		if (instance && parent && pool) {
			std::lock(get_mutex(*pool), mutex);
			std::lock_guard<std::mutex> pool_lock(get_mutex(*pool), std::adopt_lock);
			std::lock_guard<std::mutex> lock(mutex, std::adopt_lock);
			PFN_vkFree(get_instance(*parent), get_instance(*pool), 1, &instance);
		}
	}
};

// This generic is used for allocated types like command buffers.
template<typename T, typename ParentT, typename PoolT,
	VKAPI_ATTR VkResult (VKAPI_PTR *PFN_vkFree)(typename ParentT::value_type,
		typename PoolT::value_type, uint32_t, const T*)>
struct movable_allocated_with_pool_parent2 {
	template<typename U>
	friend auto get_instance(const U &value)->const decltype(value.instance)&;
	template<typename U>
	friend auto get_mutex(const U &value)->decltype(value.mutex)&;
	template<typename U>
	friend auto get_parent(U &value)->decltype(value.parent)&;
	template<typename U>
	friend auto get_parent(const U &value)->const decltype(value.parent)&;

	typedef T value_type;

protected:
	movable_allocated_with_pool_parent2() = default;
	movable_allocated_with_pool_parent2(
		const movable_allocated_with_pool_parent2 &) = delete;
	movable_allocated_with_pool_parent2(
			movable_allocated_with_pool_parent2 &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		pool = std::move(copy.pool);
		parent = std::move(copy.parent);
	}
	movable_allocated_with_pool_parent2 &operator=(
		const movable_allocated_with_pool_parent2 &) = delete;
	movable_allocated_with_pool_parent2 &operator=(
		movable_allocated_with_pool_parent2 &&copy) {
		destroy();
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		pool = std::move(copy.pool);
		parent = std::move(copy.parent);
		return *this;
	}

	~movable_allocated_with_pool_parent2() {
		destroy();
	}

	movable_allocated_with_pool_parent2(T instance,
		const type::supplier<PoolT> &pool,
		const type::supplier<ParentT> &parent)
		: instance(instance), pool(pool), parent(parent) {}

private:
	handle_type<T> instance;
	type::supplier<PoolT> pool;
	type::supplier<ParentT> parent;
	mutable std::mutex mutex;

	void destroy() {
		if (instance && pool && parent) {
			std::lock(internal::get_mutex(*pool), mutex);
			std::lock_guard<std::mutex> pool_lock(internal::get_mutex(*pool), std::adopt_lock);
			std::lock_guard<std::mutex> lock(mutex, std::adopt_lock);
			PFN_vkFree(get_instance(*parent), internal::get_instance(*pool), 1,
				&instance);
		}
	}
};

// Same as movable_destructible_with_parent but with additional memory reference.
// Used by buffer objects.
template<typename T, typename ParentT, typename MemoryT,
	void (VKAPI_PTR *PFN_vkDestroy)(
		typename ParentT::value_type, T, const VkAllocationCallbacks*)>
struct movable_destructible_with_parent_and_memory {
	template<typename U>
	friend auto get_instance(const U &value)->const decltype(value.instance)&;
	template<typename U>
	friend auto get_mutex(const U &value)->decltype(value.mutex)&;
	template<typename U>
	friend auto get_parent(U &value)->decltype(value.parent)&;
	template<typename U>
	friend auto get_parent(const U &value)->const decltype(value.parent)&;
	template<typename U>
	friend auto get_memory(U &value)->decltype(value.memory)&;
	template<typename U>
	friend VkDeviceSize &get_offset(U &value);

	typedef T value_type;

protected:
	movable_destructible_with_parent_and_memory() = default;
	movable_destructible_with_parent_and_memory(
		const movable_destructible_with_parent_and_memory &) = delete;
	movable_destructible_with_parent_and_memory(
		movable_destructible_with_parent_and_memory &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
		memory = std::move(copy.memory);
		offset = copy.offset;
	}
	movable_destructible_with_parent_and_memory &operator=(
		const movable_destructible_with_parent_and_memory &) = delete;
	movable_destructible_with_parent_and_memory &operator=(
			movable_destructible_with_parent_and_memory &&copy) {
		destroy();
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
		memory = std::move(copy.memory);
		offset = copy.offset;
		return *this;
	}

	~movable_destructible_with_parent_and_memory() {
		destroy();
	}

	movable_destructible_with_parent_and_memory(T instance,
		const type::supplier<ParentT> &parent)
		: instance(instance), parent(parent) {}

private:
	handle_type<T> instance;
	type::supplier<ParentT> parent;
	type::supplier<MemoryT> memory;
	VkDeviceSize offset;
	mutable std::mutex mutex;

	void destroy() {
		if (instance && parent) {
			std::lock_guard<std::mutex> lock(mutex);
			PFN_vkDestroy(get_instance(*parent), instance, NULL);
		}
	}
};

template<typename T, typename ParentT, typename MemoryT,
	void (VKAPI_PTR *PFN_vkDestroy)(
		typename ParentT::value_type, T, const VkAllocationCallbacks*)>
struct movable_conditional_destructible_with_parent_and_memory {
	template<typename U>
	friend auto get_instance(const U &value)->const decltype(value.instance)&;
	template<typename U>
	friend auto get_parent(U &value)->decltype(value.parent)&;
	template<typename U>
	friend auto get_parent(const U &value)->const decltype(value.parent)&;
	template<typename U>
	friend auto get_mutex(const U &value)->decltype(value.mutex)&;
	template<typename U>
	friend auto get_memory(U &value)->decltype(value.memory)&;
	template<typename U>
	friend VkDeviceSize &get_offset(U &value);

	typedef T value_type;

protected:
	movable_conditional_destructible_with_parent_and_memory() = default;
	movable_conditional_destructible_with_parent_and_memory(
		const movable_conditional_destructible_with_parent_and_memory &) = delete;
	movable_conditional_destructible_with_parent_and_memory(
			movable_conditional_destructible_with_parent_and_memory &&copy) {
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
		destructible = copy.destructible;
		memory = std::move(copy.memory);
	}
	movable_conditional_destructible_with_parent_and_memory &operator=(
		const movable_conditional_destructible_with_parent_and_memory &) = delete;
	movable_conditional_destructible_with_parent_and_memory &operator=(
			movable_conditional_destructible_with_parent_and_memory &&copy) {
		destroy();
		std::lock_guard<std::mutex> lock(copy.mutex);
		instance = std::move(copy.instance);
		parent = std::move(copy.parent);
		destructible = copy.destructible;
		memory = std::move(copy.memory);
		return *this;
	}

	~movable_conditional_destructible_with_parent_and_memory() {
		destroy();
	}

	movable_conditional_destructible_with_parent_and_memory(
		T instance, const type::supplier<ParentT> &parent,
		bool destructible = true)
		: instance(instance), parent(parent), destructible(destructible) {}

private:
	handle_type<T> instance;
	mutable std::mutex mutex;
	type::supplier<ParentT> parent;
	bool destructible;
	type::supplier<MemoryT> memory;
	VkDeviceSize offset;

	void destroy() {
		if (destructible && instance && parent) {
			std::lock_guard<std::mutex> lock(mutex);
			PFN_vkDestroy(get_instance(*parent), instance, NULL);
		}
	}
};

}  // namespace internal
}  // namespace vcc


#endif // RAII_H_
