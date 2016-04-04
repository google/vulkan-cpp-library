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
#include <type/storage.h>
#include <numeric>

TEST(ArrayTypeTest, SizeConstructor) {
	const std::size_t size(10);

	type::const_t_array<float> array(size);
	for (float f : array) {
		EXPECT_EQ(f, float());
	}
	ASSERT_EQ(size, array.size());
}

TEST(ArrayTypeTest, IteratorConstructor) {
	const std::size_t size(10);
	std::vector<float> data(size);
	std::iota(data.begin(), data.end(), 1.f);

	typedef type::const_t_array<float> array_type;
	array_type array(data.cbegin(), data.cend());
	for (std::size_t i = 0; i < data.size(); ++i) {
		EXPECT_EQ(data[i], array[i]);
	}
	ASSERT_EQ(data.size(), array.size());
}

TEST(ArrayTypeTest, InitializerConstructor) {

	type::const_t_array<float> array({1, 2, 3});
	EXPECT_EQ(1, array[0]);
	EXPECT_EQ(2, array[1]);
	EXPECT_EQ(3, array[2]);
	ASSERT_EQ(3, array.size());
}

TEST(ArrayTypeTest, MoveConstructor) {

	type::const_t_array<float> array1({1, 2, 3});
	type::const_t_array<float> array2(std::move(array1));
	EXPECT_EQ(1, array2[0]);
	EXPECT_EQ(2, array2[1]);
	EXPECT_EQ(3, array2[2]);
	ASSERT_EQ(3, array2.size());
	ASSERT_EQ(0, array1.size());
}

TEST(ArrayTypeTest, Revision) {
	type::t_array<float> array({1, 2, 3});
	ASSERT_EQ(1, type::internal::get_revision(array));
	type::mutate(array);
	ASSERT_EQ(2, type::internal::get_revision(array));
}

TEST(ArrayTypeTest, Mutate) {
	type::t_array<float> array({1, 2, 3});
	{
		type::mutable_t_array<float> mutable_array(type::mutate(array));
		std::iota(mutable_array.begin(), mutable_array.end(), 4.f);
		for (std::size_t i = 0; i < 3; ++i)
			EXPECT_EQ(float(4 + i), mutable_array[i]);
	}
	for (std::size_t i = 0; i < 3; ++i)
		EXPECT_EQ(float(4 + i), array[i]);
}

TEST(ArrayTypeTest, MutateMove) {
	type::t_array<float> array({1});
	type::mutable_t_array<float> mutable_array1(type::mutate(array));
	type::mutable_t_array<float> mutable_array2(std::move(mutable_array1));
	mutable_array2[0] = 2;
	ASSERT_EQ(2, array[0]);
}

TEST(PrimitiveTypeTest, Construct) {
	type::t_primitive<float> primitive1(1.f);
	type::t_primitive<float> primitive2;
	type::t_primitive<float> primitive3 = primitive2;
	ASSERT_EQ(1.f, primitive1);
	ASSERT_EQ(primitive2, float());
	ASSERT_EQ(primitive3, float());
}
