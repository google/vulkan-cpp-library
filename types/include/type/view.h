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

template<typename T>
struct view_lookup_supplier_container_type {
private:
	typedef typename supplier_lookup_type<T>::type supplier_value_type;
public:
	typedef typename supplier_value_type::value_type type;
};

}  // namespace internal

template<typename ContainerT>
view_type<typename internal::view_lookup_supplier_container_type<ContainerT>
	::type> make_view(ContainerT value);

template<typename T>
class view_type {
	template<typename ContainerT>
	friend view_type<typename internal::view_lookup_supplier_container_type<ContainerT>
		::type> make_view(ContainerT value);

private:
	struct read_instance_type {
		virtual ~read_instance_type() {}
		virtual const T &get(int index) const = 0;
	};

	struct instance_type {
		virtual ~instance_type() {}
		virtual std::size_t size() const = 0;
		virtual bool is_array() const = 0;
		virtual std::unique_ptr<read_instance_type> read() const = 0;
		virtual revision_type revision() const = 0;
	};

	template<typename ContainerT>
	struct read_instance_template_type : public read_instance_type {

		typedef decltype(type::read(*((ContainerT *) nullptr))) container_type;

		explicit read_instance_template_type(container_type &&container)
			: container(std::forward<container_type>(container)) {}

		const T &get(int index) const override {
			return container[index];
		}

		container_type container;
	};

	template<typename ContainerT>
	struct instance_template_type : public instance_type {
		typedef typename ContainerT::value_type value_type;
		typedef typename ContainerT::reference reference;
		typedef typename ContainerT::const_reference const_reference;
		typedef typename ContainerT::size_type size_type;

		size_type size() const override {
			return container->size();
		}

		bool is_array() const override {
			return ContainerT::is_array;
		}

		std::unique_ptr<read_instance_type> read() const override {
			return std::unique_ptr<read_instance_type>(
				new read_instance_template_type<ContainerT>(
					type::read(*container)));
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

	struct read_type {
		friend class view_type;
	public:
		read_type() = default;
		read_type(const read_type &) = delete;
		read_type(read_type &&) = default;
		read_type &operator=(const read_type &) = delete;
		read_type &operator=(read_type &&) = default;

		const T &operator[](int index) const {
			return instance->get(index);
		}

	private:
		explicit read_type(std::unique_ptr<read_instance_type> &&instance)
			: instance(
				std::forward<std::unique_ptr<read_instance_type>>(instance)) {}

		std::unique_ptr<read_instance_type> instance;
	};

	read_type read() const {
		return read_type(instance->read());
	}

	std::size_t size() const {
		return instance->size();
	}

	bool is_array() const {
		return instance->is_array();
	}

	revision_type revision() const {
		return instance->revision();
	}
};

template<typename ContainerT>
view_type<typename internal::view_lookup_supplier_container_type<ContainerT>
		::type> make_view(ContainerT value) {
	typedef typename internal::view_lookup_supplier_container_type<ContainerT>
		::type value_type;
	return view_type<value_type>(type::make_supplier(
		std::forward<ContainerT>(value)));
}

}  // namespace type

#endif // VIEW_TYPE_H_
