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
#include <type/serialize.h>

struct float2 {
	float x, y;

	bool operator==(const float2 &f) const {
		return x == f.x && y == f.y;
	}
};

struct float3 {
	float x, y, z;

	bool operator==(const float3 &f) const {
		return x == f.x && y == f.y && z == f.z;
	}
};

struct float4 {
	float x, y, z, w;

	bool operator==(const float4 &f) const {
		return x == f.x && y == f.y && z == f.z && w == f.w;
	}
};

struct mat3 {
	float3 cols[3];

	bool operator==(const mat3 &m) const {
		for (int i = 0; i < sizeof(cols) / sizeof(cols[0]); ++i) {
			if (!(cols[i] == m.cols[i])) {
				return false;
			}
		}
		return true;
	}
};

TEST(SerializeTypeTest, Constructor) {
	type::t_array<float> array({ 1, 2, 3 });
	type::serialize_type serialized(type::make_serialize(type::linear, std::ref(array)));
	ASSERT_EQ(type::size(serialized), sizeof(float) * 3);
	float output[3];
	type::flush(serialized, output);
	auto read_array(type::read(array));
	ASSERT_TRUE(std::equal(read_array.begin(), read_array.end(), output));
}

TEST(SerializeTypeTest, LinearArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<float2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<float3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	type::serialize_type serialized(type::make_serialize(type::linear,
		std::ref(array1), std::ref(array2), std::ref(array3)));
	const std::size_t size(3 + 2 * 3 + 3 * 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(), output));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(), (float2 *) (&output[0] + 3)));
	auto read_array3(type::read(array3));
	ASSERT_TRUE(std::equal(read_array3.begin(), read_array3.end(), (float3 *) (&output[0] + 3 + 6)));
}

TEST(SerializeTypeTest, LinearStd140ArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<float2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<float3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	auto view1(type::make_view(std::ref(array1)));
	auto view2(type::make_view(std::ref(array2)));
	auto view3(type::make_view(std::ref(array3)));
	type::serialize_type serialized(type::linear_std140,
			type::make_supplier(std::ref(view1)),
			type::make_supplier(std::ref(view2)),
			type::make_supplier(std::ref(view3)));
	const std::size_t size(9 * 4);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	for (std::size_t i = 0; i < array1.size(); ++i) {
		ASSERT_EQ(read_array1[i], output[i * 4]);
	}
	auto read_array2(type::read(array2));
	for (std::size_t i = 0; i < array2.size(); ++i) {
		ASSERT_EQ(read_array2[i],
			*reinterpret_cast<float2 *>(&output[12 + i * 4]));
	}
	auto read_array3(type::read(array3));
	for (std::size_t i = 0; i < array3.size(); ++i) {
		ASSERT_EQ(read_array3[i],
			*reinterpret_cast<float3 *>(&output[24 + i * 4]));
	}
}

TEST(SerializeTypeTest, LinearStd430ArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<float2> array2{ { { 1, 2 },{ 2, 3 },{ 3, 4 } } };
	type::t_array<float3> array3{ { { 1, 2, 3 },{ 4, 5, 6 },{ 7, 8, 9 } } };
	auto view1(type::make_view(std::ref(array1)));
	auto view2(type::make_view(std::ref(array2)));
	auto view3(type::make_view(std::ref(array3)));
	type::serialize_type serialized(type::linear_std430,
		type::make_supplier(std::ref(view1)),
		type::make_supplier(std::ref(view2)),
		type::make_supplier(std::ref(view3)));
	const std::size_t size(4 + 2 * 4 + 4 * 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(), output));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(),
		(float2 *)(&output[0] + 4)));
	auto read_array3(type::read(array3));
	ASSERT_EQ(read_array3[0], *((float3 *)(&output[0] + 4 + 8)));
	ASSERT_EQ(read_array3[1], *((float3 *)(&output[0] + 4 + 8 + 4)));
	ASSERT_EQ(read_array3[2], *((float3 *)(&output[0] + 4 + 8 + 8)));
}

TEST(SerializeTypeTest, LinearStd140ArrayLayout2) {
	type::t_array<float4> array1({ { 0, 10, 0, 1 } });
	type::t_array<float3> array2{ { 1, 0, 0 } };
	type::t_array<float3> array3{ { 0, 0, -1 } };
	type::t_array<float> array4{ -1 };
	type::t_array<float4> array5{ {.2f, .2f, .2f, 1} };
	type::t_array<float4> array6{ {.2f, .2f, .2f, 1} };
	type::t_array<float4> array7{ {.2f, .2f, .2f, 1} };
	type::t_array<float> array8{ { 128 } };
	type::serialize_type serialized(type::make_serialize(type::linear_std140,
		std::ref(array1), std::ref(array2), std::ref(array3), std::ref(array4),
		std::ref(array5), std::ref(array6), std::ref(array7), std::ref(array8)));
	const std::size_t size(8 * 4);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(),
		(float4 *)(&output[0] + 0)));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(),
		(float3 *)(&output[0] + 4)));
	auto read_array3(type::read(array3));
	ASSERT_TRUE(std::equal(read_array3.begin(), read_array3.end(),
		(float3 *)(&output[0] + 8)));
	auto read_array4(type::read(array4));
	ASSERT_EQ(read_array4[0], output[12]);
	auto read_array5(type::read(array5));
	ASSERT_TRUE(std::equal(read_array5.begin(), read_array5.end(),
		(float4 *)(&output[0] + 16)));
	auto read_array6(type::read(array6));
	ASSERT_TRUE(std::equal(read_array6.begin(), read_array6.end(),
		(float4 *)(&output[0] + 20)));
	auto read_array7(type::read(array7));
	ASSERT_TRUE(std::equal(read_array7.begin(), read_array7.end(),
		(float4 *)(&output[0] + 24)));
	auto read_array8(type::read(array8));
	ASSERT_EQ(read_array8[0], output[28]);
}

TEST(SerializeTypeTest, LinearStd430ArrayLayout2) {
	type::t_array<float4> array1({ { 0, 10, 0, 1 } });
	type::t_array<float3> array2{ { 1, 0, 0 } };
	type::t_array<float3> array3{ { 0, 0, -1 } };
	type::t_array<float> array4{ -1 };
	type::t_array<float4> array5{ { .2f, .2f, .2f, 1 } };
	type::t_array<float4> array6{ { .2f, .2f, .2f, 1 } };
	type::t_array<float4> array7{ { .2f, .2f, .2f, 1 } };
	type::t_array<float> array8{ { 128 } };
	type::serialize_type serialized(type::make_serialize(type::linear_std430,
		std::ref(array1), std::ref(array2), std::ref(array3), std::ref(array4),
		std::ref(array5), std::ref(array6), std::ref(array7), std::ref(array8)));
	const std::size_t size(4 * 7 + 1);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(),
		(float4 *)(&output[0] + 0)));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(),
		(float3 *)(&output[0] + 4)));
	auto read_array3(type::read(array3));
	ASSERT_TRUE(std::equal(read_array3.begin(), read_array3.end(),
		(float3 *)(&output[0] + 8)));
	auto read_array4(type::read(array4));
	ASSERT_EQ(read_array4[0], output[12]);
	auto read_array5(type::read(array5));
	ASSERT_TRUE(std::equal(read_array5.begin(), read_array5.end(),
		(float4 *)(&output[0] + 16)));
	auto read_array6(type::read(array6));
	ASSERT_TRUE(std::equal(read_array6.begin(), read_array6.end(),
		(float4 *)(&output[0] + 20)));
	auto read_array7(type::read(array7));
	ASSERT_TRUE(std::equal(read_array7.begin(), read_array7.end(),
		(float4 *)(&output[0] + 24)));
	auto read_array8(type::read(array8));
	ASSERT_EQ(read_array8[0], output[28]);
}


TEST(SerializeTypeTest, LinearPrimitiveLayout1) {
	type::t_primitive<float> primitive1(1);
	type::t_primitive<float2> primitive2({3, 4});
	type::t_primitive<float3> primitive3({4, 5, 6});
	auto view1(type::make_view(std::ref(primitive1)));
	auto view2(type::make_view(std::ref(primitive2)));
	auto view3(type::make_view(std::ref(primitive3)));
	type::serialize_type serialized(type::linear_std140,
			type::make_supplier(std::ref(view1)),
			type::make_supplier(std::ref(view2)),
			type::make_supplier(std::ref(view3)));
	const std::size_t size(4 + 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	ASSERT_EQ(type::read(primitive1), output[0]);
	ASSERT_EQ(type::read(primitive2), *((float2 *) &output[2]));
	ASSERT_EQ(type::read(primitive3), *((float3 *) &output[4]));
}

TEST(SerializeTypeTest, InterleavedStd140ArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<float2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<float3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	auto view1(type::make_view(std::ref(array1)));
	auto view2(type::make_view(std::ref(array2)));
	auto view3(type::make_view(std::ref(array3)));
	type::serialize_type serialized(type::interleaved_std140,
			type::make_supplier(std::ref(view1)),
			type::make_supplier(std::ref(view2)),
			type::make_supplier(std::ref(view3)));
	const std::size_t size(12 * 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	for (std::size_t i = 0; i < array1.size(); ++i) {
		ASSERT_EQ(read_array1[i], output[i * 12]);
	}
	auto read_array2(type::read(array2));
	for (std::size_t i = 0; i < array2.size(); ++i) {
		ASSERT_EQ(read_array2[i],
			*reinterpret_cast<float2 *>(&output[4 + i * 12]));
	}
	auto read_array3(type::read(array3));
	for (std::size_t i = 0; i < array3.size(); ++i) {
		ASSERT_EQ(read_array3[i],
			*reinterpret_cast<float3 *>(&output[8 + i * 12]));
	}
}

TEST(SerializeTypeTest, GroupedInterleavedStd140ArrayLayout1) {
	type::t_array<float> array1({ 1, 2 });
	type::t_array<float2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<float3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	type::t_array<float3> array4{{ {10, 11, 12}, {13, 14, 15}, {16, 17, 18}, {19, 20, 21} }};
	auto view1(type::make_view(std::ref(array1)));
	auto view2(type::make_view(std::ref(array2)));
	auto view3(type::make_view(std::ref(array3)));
	auto view4(type::make_view(std::ref(array4)));
	type::serialize_type serialized(type::interleaved_std140,
			type::make_supplier(std::ref(view1)),
			type::make_supplier(std::ref(view2)),
			type::make_supplier(std::ref(view3)),
			type::make_supplier(std::ref(view4)));
	const std::size_t size(8 + 8 * 3 + 4 * 4);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	const float compare1[] = { 1, 2 };
	const float compare2[] = { 1, 2, 3 };
	const float compare3[] = { 2, 3 };
	const float compare4[] = { 4, 5, 6 };
	const float compare5[] = { 3, 4 };
	const float compare6[] = { 7, 8, 9 };
	const float compare7[] = { 10, 11, 12 };
	const float compare8[] = { 13, 14, 15 };
	const float compare9[] = { 16, 17, 18 };
	const float compare10[] = { 19, 20, 21 };
	ASSERT_EQ(1, output[0]);
	ASSERT_EQ(2, output[4]);
	ASSERT_TRUE(std::equal(&output[0] + 8, &output[0] + 10, compare1));
	ASSERT_TRUE(std::equal(&output[0] + 12, &output[0] + 15, compare2));
	ASSERT_TRUE(std::equal(&output[0] + 16, &output[0] + 18, compare3));
	ASSERT_TRUE(std::equal(&output[0] + 20, &output[0] + 23, compare4));
	ASSERT_TRUE(std::equal(&output[0] + 24, &output[0] + 26, compare5));
	ASSERT_TRUE(std::equal(&output[0] + 28, &output[0] + 31, compare6));
	ASSERT_TRUE(std::equal(&output[0] + 32, &output[0] + 35, compare7));
	ASSERT_TRUE(std::equal(&output[0] + 36, &output[0] + 39, compare8));
	ASSERT_TRUE(std::equal(&output[0] + 40, &output[0] + 43, compare9));
	ASSERT_TRUE(std::equal(&output[0] + 44, &output[0] + 47, compare10));
}
