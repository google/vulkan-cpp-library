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
#ifndef VIEW_TYPE_H_
#define VIEW_TYPE_H_

#include <type/storage.h>
#include <type/revision.h>
#include <type/supplier.h>

namespace type {

template<typename T>
class view_type;

namespace internal {

template<typename T, typename ContainerT>
auto make_view(T value)->view_type<typename ContainerT::value_type>;

}  // namespace internal

template<typename T>
class view_type {
	template<typename U, typename ContainerT>
	friend auto internal::make_view(U value)->view_type<typename ContainerT::value_type>;

private:
	struct instance_type {
		virtual ~instance_type() {}
		virtual std::size_t size() const = 0;
		virtual bool is_array() const = 0;
		virtual const T &get(int index) const = 0;
		virtual void lock() = 0;
		virtual void unlock() = 0;
		virtual revision_type revision() const = 0;
	};

	template<typename ContainerT>
	struct instance_template_type : public instance_type {
		typedef typename ContainerT::value_type value_type;
		typedef typename ContainerT::reference reference;
		typedef typename ContainerT::const_reference const_reference;
		typedef typename ContainerT::size_type size_type;
		typedef typename ContainerT::lock_type lock_type;

		lock_type container_lock;

		std::size_t size() const override {
			return container->size();
		}

		bool is_array() const override {
			return ContainerT::is_array;
		}

		const T &get(int index) const override {
			return (*container)[index];
		}

		void lock() override {
			container_lock = internal::get_lock(*container);
		}

		void unlock() override {
			container_lock = lock_type();
		}

		revision_type revision() const override {
			return internal::get_revision(*container);
		}

		explicit instance_template_type(const supplier<ContainerT> &container)
			: container(container) {}
		supplier<ContainerT> container;
	};

	template<typename ContainerT>
	explicit view_type(const supplier<ContainerT> &container)
		: instance(new instance_template_type<ContainerT>(container)) {}

	void unlock() {
		instance->unlock();
	}

	std::unique_ptr<instance_type> instance;

public:

	view_type(const view_type&) = delete;
	view_type(view_type&&) = default;
	view_type &operator=(const view_type&) = delete;
	view_type &operator=(view_type&&) = default;

	struct lock_type {
		friend class view_type;
	public:
		lock_type() : view(nullptr) {}
		lock_type(const lock_type&) = delete;
		lock_type(lock_type &&copy) : view(copy.view) {
			copy.view = nullptr;
		}
		lock_type &operator=(const lock_type&) = delete;
		lock_type &operator=(lock_type&&copy) {
			release();
			view = copy.view;
			copy.view = nullptr;
			return *this;
		}
		~lock_type() {
			release();
		}
	private:
		explicit lock_type(view_type *view) : view(view) {}
		void release() {
			if (view) {
				view->unlock();
			}
		}
		view_type *view;
	};

	lock_type lock() {
		return lock_type(this);
	}

	std::size_t size() const {
		return instance->size();
	}

	bool is_array() const {
		return instance->is_array();
	}

	const T &operator[](int index) const {
		return instance->get(index);
	}

	revision_type revision() const {
		return instance->revision();
	}
};

namespace internal {

template<typename T, typename ContainerT>
auto make_view(T container)->view_type<typename ContainerT::value_type> {
	return view_type<typename ContainerT::value_type>(make_supplier(
		std::forward<T>(container)));
}

}  // namespace internal

template<typename T>
auto make_view(T value)->decltype(internal::make_view<T, typename decltype(
		make_supplier(std::forward<T>(value)))::value_type>(value)) {
	typedef decltype(make_supplier(std::forward<T>(value))) supplier_type;
	return internal::make_view<T, typename supplier_type::value_type>(value);
}

}  // namespace type

#endif // VIEW_TYPE_H_
