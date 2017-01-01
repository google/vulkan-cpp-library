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
#include <gtest/gtest.h>
#include <type/transform.h>

struct float2 {
	float x, y;

	bool operator==(const float2 &f) const {
		return x == f.x && y == f.y;
	}
};

TEST(TransformTypeTest, Constructor) {
	type::t_array<float> array({1, 2, 3});
	auto transform(type::make_transform(type::t_array<float2>(array.size()),
		[](const type::readable_t_array<float, true> &input, type::writable_t_array<float2> &&output) {
			std::transform(std::begin(input), std::end(input), std::begin(output),
				[](float value) {
					return float2{ 0, 0 };
			});
		}, std::ref(array)));
	ASSERT_EQ(transform.size(), array.size());
}

TEST(TransformTypeTest, Iterate) {
	type::t_array<float> array({1, 2, 3});
	auto transform(type::make_transform(type::t_array<float2>(array.size()),
		[](const type::readable_t_array<float, true> &input, type::writable_t_array<float2> &&output) {
			std::transform(std::begin(input), std::end(input), std::begin(output),
				[](float value) {
					return float2{ value, -value };
			});
		}, std::ref(array)));
	auto read_transform(type::read(transform));
	for (std::size_t i = 0; i < array.size(); ++i) {
		const float2 f{ float(i + 1), -float(i + 1) };
		EXPECT_EQ(f, read_transform[i]);
	}
}

TEST(TransformTypeTest, Mutate) {
	type::t_array<float> array({1, 2, 3});
	auto transform(type::make_transform(type::t_array<float2>(array.size()),
		[](const type::readable_t_array<float, true> &input, type::writable_t_array<float2> && output) {
			std::transform(std::begin(input), std::end(input), std::begin(output),
				[](float value) {
				return float2{ value, -value };
			});
		}, std::ref(array)));
	for (float &f : type::write(array)) {
		f += 1;
	}
	auto transform_read(type::read(transform));
	for (std::size_t i = 0; i < array.size(); ++i) {
		float2 f{ float(i + 2), -float(i + 2) };
		EXPECT_EQ(f, transform_read[i]);
	}
}

TEST(TransformTypeTest, MultipleArguments) {
	type::t_array<float> array1({ 1, 2, 3 }), array2({ 4, 5, 6 });
	type::t_primitive<float> primitive(1);
	auto transform(type::make_transform(type::t_array<float2>(array1.size()),
		[](const type::readable_t_array<float, true> &input1,
				const type::readable_t_array<float, true> &input2,
				const type::readable_t_primitive<float, true> &input3,
				type::writable_t_array<float2> &&output) {
			std::transform(std::begin(input1), std::end(input1), std::begin(input2),
				std::begin(output),
				[&](float value1, float value2) {
					return float2{ value1, -value2 + input3[0] };
				});
		}, std::ref(array1), std::ref(array2), std::ref(primitive)));
	auto transform_read(type::read(transform));
	for (std::size_t i = 0; i < array1.size(); ++i) {
		float2 f{ float(i + 1), -float(i + 4) + 1 };
		EXPECT_EQ(f, transform_read[i]);
	}
}

TEST(TransformTypeTest, RedundantFlush) {
	type::t_array<float> array({ 1, 2, 3 });
	int counter(0);
	auto transform(type::make_transform(type::t_array<float2>(array.size()),
		[&](const type::readable_t_array<float, true> &input, type::writable_t_array<float2> &&output) {
		std::transform(std::begin(input), std::end(input), std::begin(output),
			[](float value) {
			return float2{ value, -value };
		});
		++counter;
	}, std::ref(array)));
	{
		auto transform_read(type::read(transform));
		for (std::size_t i = 0; i < array.size(); ++i) {
			float2 f{ float(i + 1), -float(i + 1) };
			EXPECT_EQ(f, transform_read[i]);
		}
	}
	{
		auto transform_read(type::read(transform));
		for (std::size_t i = 0; i < array.size(); ++i) {
			float2 f{ float(i + 1), -float(i + 1) };
			EXPECT_EQ(f, transform_read[i]);
		}
	}
	ASSERT_EQ(counter, 1);
}
