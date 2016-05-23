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
#ifndef _VCC_INTERNAL_HOOK_H_
#define _VCC_INTERNAL_HOOK_H_

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace vcc {
namespace internal {

template<typename... T>
class hook_container_type {
public:
	typedef std::function<void(T...)> callback_type;
	hook_container_type() = default;
	hook_container_type(const hook_container_type&) = delete;
	hook_container_type(hook_container_type&&) = default;
	hook_container_type &operator=(const hook_container_type&) = delete;
	hook_container_type &operator=(hook_container_type&&) = default;

	void add(const callback_type &callback) {
		callbacks.push_back(callback);
	}
	void add(callback_type &&callback) {
		callbacks.push_back(std::forward<callback_type>(callback));
	}

	void operator() (T... value) const {
		for (const callback_type &callback : callbacks) {
			callback(value...);
		}
	}
private:
	std::vector<callback_type> callbacks;
};

template<typename KeyT, typename Hash, typename... T>
class hook_map_type {
public:
	typedef std::function<void(T...)> callback_type;
	hook_map_type() = default;
	hook_map_type(const hook_map_type&) = delete;
	hook_map_type(hook_map_type&&) = default;
	hook_map_type &operator=(const hook_map_type&) = delete;
	hook_map_type &operator=(hook_map_type&&) = default;

	void put(const KeyT &key, callback_type &&callback) {
		callbacks.emplace(key, std::forward<callback_type>(callback));
	}

	void operator() (T... value) const {
		for (const typename callbacks_container_type::value_type &callback
				: callbacks) {
			callback.second(value...);
		}
	}
private:
	typedef std::unordered_map<KeyT, callback_type, Hash> callbacks_container_type;
	callbacks_container_type callbacks;
};

class reference_container_type {
private:
	struct instance {
		virtual ~instance() {}
	};
	template<typename... T>
	struct template_instance : public instance {
		template_instance(T... value) : value(std::forward<T>(value)...) {}
		template_instance(const std::tuple<T...> &value) : value(value) {}
		std::tuple<T...> value;
	};
public:
	reference_container_type() = default;
	reference_container_type(const reference_container_type &) = delete;
	reference_container_type(reference_container_type &&) = default;
	reference_container_type &operator=(const reference_container_type &) = delete;
	reference_container_type &operator=(reference_container_type &&) = default;

	template<typename... T>
	void add(T... value) {
		instances.emplace_back(new template_instance<T...>(std::forward<T>(value)...));
	}
private:
	std::vector<std::shared_ptr<instance>> instances;
};

template<typename KeyT, typename HashT = std::hash<KeyT>>
class reference_map_type {
private:
	struct instance {
		virtual ~instance() {}
		virtual std::shared_ptr<instance> clone() const = 0;
	};
	template<typename... T>
	struct template_instance : public instance {
		template_instance(T... value) : value(std::forward<T>(value)...) {}
		template_instance(const std::tuple<T...> &value) : value(value) {}
		std::shared_ptr<instance> clone() const {
			return std::make_shared<template_instance<T...>>(value);
		}
		std::tuple<T...> value;
	};
public:
	reference_map_type() = default;
	reference_map_type(const reference_map_type &) = delete;
	reference_map_type(reference_map_type &&) = default;
	reference_map_type &operator=(const reference_map_type &) = delete;
	reference_map_type &operator=(reference_map_type &&) = default;

	template<typename... T>
	void put(const KeyT &key, T... value) {
		instances.emplace(key, std::shared_ptr<instance>(new template_instance<T...>(std::forward<T>(value)...)));
	}

	void clone(const KeyT &key, const reference_map_type &map) {
		put(key, map.instances.at(key)->clone());
	}
private:
	std::unordered_map<KeyT, std::shared_ptr<instance>, HashT> instances;
};

}  // namespace internal
}  // namespace vcc

#endif // _VCC_INTERNAL_HOOK_H_
