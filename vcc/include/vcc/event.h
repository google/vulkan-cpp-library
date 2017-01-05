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
#ifndef EVENT_H_
#define EVENT_H_

#include <vcc/device.h>

namespace vcc {
namespace event {

struct event_type : internal::movable_destructible_with_parent<VkEvent, const device::device_type,
		vkDestroyEvent> {
	friend VCC_LIBRARY event_type create(const type::supplier<const device::device_type> &device);

	event_type() = default;
	event_type(event_type &&) = default;
	event_type(const event_type &) = delete;
	event_type &operator=(event_type &&) = default;
	event_type &operator=(const event_type &) = delete;

private:
	event_type(VkEvent instance, const type::supplier<const device::device_type> &parent)
		: movable_destructible_with_parent(instance, parent) {}
};

VCC_LIBRARY event_type create(const type::supplier<const device::device_type> &device);
VCC_LIBRARY VkResult status(const event_type &event);
VCC_LIBRARY void set(const event_type &event);
VCC_LIBRARY void reset(const event_type &event);

}  // namespace event
}  // namespace vcc

#endif /* EVENT_H_ */
