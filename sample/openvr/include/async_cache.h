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
#ifndef _OPENVR_ASYNC_CACHE_H_
#define _OPENVR_ASYNC_CACHE_H_

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

template<typename KeyT, typename ValueT, typename ExecutorT>
struct async_cache_type {
public:
	typedef std::function<void(const KeyT &, std::shared_ptr<ValueT> &&)> callback_type;
	typedef std::function<ValueT(const KeyT &)> loader_type;
private:
	typedef std::vector<callback_type> callbacks_container_type;
	typedef std::unordered_map<KeyT, callbacks_container_type> pending_map_type;
	typedef std::unordered_map<KeyT, std::weak_ptr<ValueT>> value_map_type;
public:

	async_cache_type(ExecutorT &&executor, loader_type &&loader)
		: executor(std::forward<ExecutorT>(executor)),
		loader(std::forward<loader_type>(loader)) {}

	void put(const KeyT &key, callback_type &&callback) {
		{
			std::unique_lock<std::mutex> lk(map_mutex);
			const auto value_it = values_map.find(key);
			if (value_it != values_map.end()) {
				try {
					callback(key, std::shared_ptr<ValueT>(value_it->second));
					return;
				}
				catch (std::bad_weak_ptr &) {
					values_map.erase(value_it);
				}
			}
			auto &callbacks(pendings_map[key]);
			const bool empty(callbacks.empty());
			callbacks.push_back(std::forward<callback_type>(callback));
			if (!empty) {
				return;
			}
		}
		executor(std::bind([this, key]() {
			auto value(std::make_shared<ValueT>(loader(key)));
			std::vector<callback_type> callbacks;
			{
				std::unique_lock<std::mutex> lk(map_mutex);
				values_map.emplace(key, value);
				callbacks = std::move(pendings_map.at(key));
			}
			for (const auto &callback : callbacks) {
				callback(key, std::move(value));
			}
		}));
	}

private:
	ExecutorT executor;
	loader_type loader;
	value_map_type values_map;
	pending_map_type pendings_map;
	std::mutex map_mutex;
};

#endif  // _OPENVR_ASYNC_CACHE_H_
