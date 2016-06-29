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
#ifndef _OPENVR_THREAD_POOL_H_
#define _OPENVR_THREAD_POOL_H_

#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>

struct thread_pool_type {
	typedef std::function<void()> task_type;
private:
	class instance_type {
	public:
		explicit instance_type(size_t num_threads) : threads(num_threads) {
			running = true;
			for (std::thread &thread : threads) {
				thread = std::thread([this]() {
					for (;;) {
						task_type task;
						{
							std::unique_lock<std::mutex> lk(tasks_mutex);
							tasks_cv.wait(lk, [this] {
								return !tasks.empty() || !running; });
							if (!running) {
								break;
							}
							if (tasks.empty()) {
								continue;
							}
							task = std::move(tasks.front());
							tasks.pop();
						}
						task();
					}
				});
			}
		}

		void operator()(task_type &&task) {
			{
				std::unique_lock<std::mutex> lk(tasks_mutex);
				tasks.push(std::forward<task_type>(task));
			}
			tasks_cv.notify_one();
		}

		~instance_type() {
			running = false;
			tasks_cv.notify_all();
			for (std::thread &thread : threads) {
				if (thread.joinable()) {
					thread.join();
				}
			}
		}

	private:
		std::vector<std::thread> threads;
		std::queue<task_type> tasks;
		std::mutex tasks_mutex;
		std::condition_variable tasks_cv;
		volatile bool running;
	};
public:
	explicit thread_pool_type(size_t num_threads)
		: instance(new instance_type(num_threads)) {}

	thread_pool_type() = default;
	thread_pool_type(const thread_pool_type &) = delete;
	thread_pool_type &operator=(const thread_pool_type &) = delete;
	thread_pool_type(thread_pool_type &&thread_pool) = default;
	thread_pool_type &operator=(thread_pool_type &&) = default;

	void operator()(task_type &&task) const {
		instance->operator()(std::forward<task_type>(task));
	}

private:
	std::unique_ptr<instance_type> instance;
};

#endif  // _OPENVR_THREAD_POOL_H_
