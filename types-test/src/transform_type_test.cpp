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
	type::transform_type<float2> transform(type::make_transform(std::ref(array), [](float f){
		return float2{0, 0};
	}));
	ASSERT_EQ(transform.size(), array.size());
}

TEST(TransformTypeTest, Iterate) {
	type::t_array<float> array({1, 2, 3});
	type::transform_type<float2> transform(type::make_transform(std::ref(array), [](float f){
		return float2{f, -f};
	}));
	auto read_transform(type::read(transform));
	for (std::size_t i = 0; i < array.size(); ++i) {
		const float2 f{ float(i + 1), -float(i + 1) };
		EXPECT_EQ(f, read_transform[i]);
	}
}

TEST(TransformTypeTest, Mutate) {
	type::t_array<float> array({1, 2, 3});
	type::transform_type<float2> transform(type::make_transform(std::ref(array), [](float f){
		return float2{f, -f};
	}));
	for (float &f : type::write(array)) {
		f += 1;
	}
	auto transform_read(type::read(transform));
	for (std::size_t i = 0; i < array.size(); ++i) {
		float2 f{ float(i + 2), -float(i + 2) };
		EXPECT_EQ(f, transform_read[i]);
	}
}
