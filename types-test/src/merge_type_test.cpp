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
#include <type/merge.h>
#include <type/storage.h>

TEST(MergeTypeTest, Constructor) {
	type::t_array<float> array1({1, 2, 3});
	type::t_array<float> array2({4, 5, 6});
	type::merge_type<float> merge(
		type::make_merge(std::ref(array1), std::ref(array2)));
	ASSERT_EQ(merge.size(), array1.size() + array2.size());
}

TEST(MergeTypeTest, Index) {
	type::t_array<float> array1({1, 2, 3});
	type::t_array<float> array2({4, 5, 6});
	type::merge_type<float> merge(
		type::make_merge(std::ref(array1), std::ref(array2)));
	for (std::size_t i = 0; i < array1.size(); ++i) {
		EXPECT_EQ(array1[i], merge[i]);
	}
	for (std::size_t i = 0; i < array2.size(); ++i) {
		EXPECT_EQ(array2[i], merge[array1.size() + i]);
	}
}

TEST(MergeTypeTest, Iterator) {
	type::t_array<float> array1({1, 2, 3});
	type::t_array<float> array2({4, 5, 6, 7});
	type::merge_type<float> merge(
		type::make_merge(std::ref(array1), std::ref(array2)));
	for (auto it(merge.begin()); it != merge.begin() + array1.size(); ++it) {
		EXPECT_EQ(*it, array1[std::distance(merge.begin(), it)]);
	}
	for (auto it(merge.begin() + array1.size()); it != merge.end(); ++it) {
		EXPECT_EQ(*it,
			array2[std::distance(merge.begin() + array1.size(), it)]);
	}
}

TEST(MergeTypeTest, Mutate) {
	type::t_array<float> array1({1, 2, 3});
	type::t_array<float> array2({4, 5, 6, 7});
	type::merge_type<float> merge(
		type::make_merge(std::ref(array1), std::ref(array2)));
	for (float &f : type::mutate(array1)) {
		f += 1;
	}
	for (auto it(merge.begin()); it != merge.begin() + array1.size(); ++it) {
		EXPECT_EQ(*it, array1[std::distance(merge.begin(), it)]);
	}
	for (auto it(merge.begin() + array1.size()); it != merge.end(); ++it) {
		EXPECT_EQ(*it,
			array2[std::distance(merge.begin() + array1.size(), it)]);
	}
}
