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
#include <type/view.h>

TEST(ViewTypeTest, Constructor) {
	type::const_t_array<float> array({ 1, 2, 3 });
	type::view_type<float> view(type::make_view(std::ref(array)));
	ASSERT_EQ(3, view.size());
}

TEST(ViewTypeTest, Read) {
	type::const_t_array<float> array({ 1, 2, 3 });
	type::view_type<float> view(type::make_view(std::ref(array)));
	auto read_view(view.read());
	ASSERT_EQ(3, view.size());
	EXPECT_EQ(1, read_view[0]);
	EXPECT_EQ(2, read_view[1]);
	EXPECT_EQ(3, read_view[2]);
}

TEST(ViewTypeTest, Revision) {
	type::t_array<float> array({ 1, 2, 3 });
	type::view_type<float> view(type::make_view(std::ref(array)));
	ASSERT_EQ(1, view.revision());
	type::write(array)[1] = 4;
	ASSERT_EQ(2, view.revision());
	ASSERT_EQ(4, view.read()[1]);
}
