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

TEST(SerializeTypeTest, Constructor) {
	type::t_array<float> array({ 1, 2, 3 });
	type::serialize_type serialized(type::make_serialize<type::linear>(
		type::make_supplier(std::ref(array))));
	ASSERT_EQ(type::size(serialized), sizeof(float) * 3);
	float output[3];
	type::flush(serialized, output);
	auto read_array(type::read(array));
	ASSERT_TRUE(std::equal(read_array.begin(), read_array.end(), output));
}

TEST(SerializeTypeTest, LinearArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<glm::vec2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<glm::vec3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	type::serialize_type serialized(type::make_serialize<type::linear>(
		type::make_supplier(std::ref(array1)), type::make_supplier(std::ref(array2)),
		type::make_supplier(std::ref(array3))));
	const std::size_t size(3 + 2 * 3 + 3 * 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(), output));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(), (glm::vec2 *) (&output[0] + 3)));
	auto read_array3(type::read(array3));
	ASSERT_TRUE(std::equal(read_array3.begin(), read_array3.end(), (glm::vec3 *) (&output[0] + 3 + 6)));
}

TEST(SerializeTypeTest, LinearStd140ArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<glm::vec2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<glm::vec3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	auto serialized(type::make_serialize<type::linear_std140>(
			type::make_supplier(std::ref(array1)),
			type::make_supplier(std::ref(array2)),
			type::make_supplier(std::ref(array3))));
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
			*reinterpret_cast<glm::vec2 *>(&output[12 + i * 4]));
	}
	auto read_array3(type::read(array3));
	for (std::size_t i = 0; i < array3.size(); ++i) {
		ASSERT_EQ(read_array3[i],
			*reinterpret_cast<glm::vec3 *>(&output[24 + i * 4]));
	}
}

TEST(SerializeTypeTest, LinearStd430ArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<glm::vec2> array2{ { { 1, 2 },{ 2, 3 },{ 3, 4 } } };
	type::t_array<glm::vec3> array3{ { { 1, 2, 3 },{ 4, 5, 6 },{ 7, 8, 9 } } };
	auto serialized(type::make_serialize<type::linear_std430>(
		type::make_supplier(std::ref(array1)),
		type::make_supplier(std::ref(array2)),
		type::make_supplier(std::ref(array3))));
	const std::size_t size(4 + 2 * 4 + 4 * 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(), output));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(),
		(glm::vec2 *)(&output[4])));
	auto read_array3(type::read(array3));
	ASSERT_EQ(read_array3[0], *((glm::vec3 *)(&output[4 + 8])));
	ASSERT_EQ(read_array3[1], *((glm::vec3 *)(&output[4 + 8 + 4])));
	ASSERT_EQ(read_array3[2], *((glm::vec3 *)(&output[4 + 8 + 8])));
}

TEST(SerializeTypeTest, LinearStd140ArrayLayout2) {
	type::t_array<glm::vec4> array1({ { 0, 10, 0, 1 } });
	type::t_array<glm::vec3> array2{ { 1, 0, 0 } };
	type::t_array<glm::vec3> array3{ { 0, 0, -1 } };
	type::t_array<float> array4{ -1 };
	type::t_array<glm::vec4> array5{ {.2f, .2f, .2f, 1} };
	type::t_array<glm::vec4> array6{ {.2f, .2f, .2f, 1} };
	type::t_array<glm::vec4> array7{ {.2f, .2f, .2f, 1} };
	type::t_array<float> array8{ { 128 } };
	type::serialize_type serialized(type::make_serialize<type::linear_std140>(
		type::make_supplier(std::ref(array1)), type::make_supplier(std::ref(array2)),
		type::make_supplier(std::ref(array3)), type::make_supplier(std::ref(array4)),
		type::make_supplier(std::ref(array5)), type::make_supplier(std::ref(array6)),
		type::make_supplier(std::ref(array7)), type::make_supplier(std::ref(array8))));
	const std::size_t size(8 * 4);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(),
		(glm::vec4 *)(&output[0] + 0)));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(),
		(glm::vec3 *)(&output[0] + 4)));
	auto read_array3(type::read(array3));
	ASSERT_TRUE(std::equal(read_array3.begin(), read_array3.end(),
		(glm::vec3 *)(&output[0] + 8)));
	auto read_array4(type::read(array4));
	ASSERT_EQ(read_array4[0], output[12]);
	auto read_array5(type::read(array5));
	ASSERT_TRUE(std::equal(read_array5.begin(), read_array5.end(),
		(glm::vec4 *)(&output[0] + 16)));
	auto read_array6(type::read(array6));
	ASSERT_TRUE(std::equal(read_array6.begin(), read_array6.end(),
		(glm::vec4 *)(&output[0] + 20)));
	auto read_array7(type::read(array7));
	ASSERT_TRUE(std::equal(read_array7.begin(), read_array7.end(),
		(glm::vec4 *)(&output[0] + 24)));
	auto read_array8(type::read(array8));
	ASSERT_EQ(read_array8[0], output[28]);
}

TEST(SerializeTypeTest, LinearStd430ArrayLayout2) {
	type::t_array<glm::vec4> array1({ { 0, 10, 0, 1 } });
	type::t_array<glm::vec3> array2{ { 1, 0, 0 } };
	type::t_array<glm::vec3> array3{ { 0, 0, -1 } };
	type::t_array<float> array4{ -1 };
	type::t_array<glm::vec4> array5{ { .2f, .2f, .2f, 1 } };
	type::t_array<glm::vec4> array6{ { .2f, .2f, .2f, 1 } };
	type::t_array<glm::vec4> array7{ { .2f, .2f, .2f, 1 } };
	type::t_array<float> array8{ { 128 } };
	type::serialize_type serialized(type::make_serialize<type::linear_std430>(
		type::make_supplier(std::ref(array1)), type::make_supplier(std::ref(array2)),
		type::make_supplier(std::ref(array3)), type::make_supplier(std::ref(array4)),
		type::make_supplier(std::ref(array5)), type::make_supplier(std::ref(array6)),
		type::make_supplier(std::ref(array7)), type::make_supplier(std::ref(array8))));
	const std::size_t size(4 * 7 + 1);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	ASSERT_TRUE(std::equal(read_array1.begin(), read_array1.end(),
		(glm::vec4 *)(&output[0] + 0)));
	auto read_array2(type::read(array2));
	ASSERT_TRUE(std::equal(read_array2.begin(), read_array2.end(),
		(glm::vec3 *)(&output[0] + 4)));
	auto read_array3(type::read(array3));
	ASSERT_TRUE(std::equal(read_array3.begin(), read_array3.end(),
		(glm::vec3 *)(&output[0] + 8)));
	auto read_array4(type::read(array4));
	ASSERT_EQ(read_array4[0], output[12]);
	auto read_array5(type::read(array5));
	ASSERT_TRUE(std::equal(read_array5.begin(), read_array5.end(),
		(glm::vec4 *)(&output[0] + 16)));
	auto read_array6(type::read(array6));
	ASSERT_TRUE(std::equal(read_array6.begin(), read_array6.end(),
		(glm::vec4 *)(&output[0] + 20)));
	auto read_array7(type::read(array7));
	ASSERT_TRUE(std::equal(read_array7.begin(), read_array7.end(),
		(glm::vec4 *)(&output[0] + 24)));
	auto read_array8(type::read(array8));
	ASSERT_EQ(read_array8[0], output[28]);
}

TEST(SerializeTypeTest, LinearStd140MatrixLayout1) {
	type::t_array<glm::mat4> array1({
		glm::mat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16),
		glm::mat4(17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32)
	});
	type::t_array<glm::mat3> array2({
		glm::mat3(33, 34, 35, 36, 37, 38, 39, 40, 41),
		glm::mat3(42, 43, 44, 45, 46, 47, 48, 49, 50)
	});
	auto serialized(type::make_serialize<type::linear_std140>(
		type::make_supplier(std::ref(array1)),
		type::make_supplier(std::ref(array2))));
	const std::size_t size(4 * 4 * 2 + 4 * 3 * 2);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	for (std::size_t i = 0; i < 35; ++i) {
		ASSERT_EQ(output[i], i + 1);
	}
	for (std::size_t i = 36; i < 39; ++i) {
		ASSERT_EQ(output[i], i);
	}
	for (std::size_t i = 40; i < 43; ++i) {
		ASSERT_EQ(output[i], i - 1);
	}
	for (std::size_t i = 44; i < 47; ++i) {
		ASSERT_EQ(output[i], i - 2);
	}
	for (std::size_t i = 48; i < 51; ++i) {
		ASSERT_EQ(output[i], i - 3);
	}
	for (std::size_t i = 52; i < 55; ++i) {
		ASSERT_EQ(output[i], i - 4);
	}
}

TEST(SerializeTypeTest, LinearPrimitiveLayout1) {
	type::t_primitive<float> primitive1(1);
	type::t_primitive<glm::vec2> primitive2({3, 4});
	type::t_primitive<glm::vec3> primitive3({4, 5, 6});
	auto serialized(type::make_serialize<type::linear_std140>(
			type::make_supplier(std::ref(primitive1)),
			type::make_supplier(std::ref(primitive2)),
			type::make_supplier(std::ref(primitive3))));
	const std::size_t size(4 + 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	ASSERT_EQ(type::read(primitive1), output[0]);
	ASSERT_EQ(type::read(primitive2), *((glm::vec2 *) &output[2]));
	ASSERT_EQ(type::read(primitive3), *((glm::vec3 *) &output[4]));
}

TEST(SerializeTypeTest, InterleavedStd140ArrayLayout1) {
	type::t_array<float> array1({ 1, 2, 3 });
	type::t_array<glm::vec2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<glm::vec3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	auto serialized(type::make_serialize<type::interleaved_std140>(
			type::make_supplier(std::ref(array1)),
			type::make_supplier(std::ref(array2)),
			type::make_supplier(std::ref(array3))));
	const std::size_t size(8 * 3);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	auto read_array1(type::read(array1));
	for (std::size_t i = 0; i < array1.size(); ++i) {
		ASSERT_EQ(read_array1[i], output[i * 8]);
	}
	auto read_array2(type::read(array2));
	for (std::size_t i = 0; i < array2.size(); ++i) {
		ASSERT_EQ(read_array2[i],
			*reinterpret_cast<glm::vec2 *>(&output[2 + i * 8]));
	}
	auto read_array3(type::read(array3));
	for (std::size_t i = 0; i < array3.size(); ++i) {
		ASSERT_EQ(read_array3[i],
			*reinterpret_cast<glm::vec3 *>(&output[4 + i * 8]));
	}
}

TEST(SerializeTypeTest, InterleavedStd140ArrayLayout2) {
	type::t_array<glm::vec3> array1{ { 1, 2, 3 }, { 4, 5, 6 } };
	type::t_array<glm::vec2> array2{ { 7, 8 }, { 9, 10 } };
	type::t_array<glm::mat3> array3{ { 11, 12, 13, 14, 15, 16, 17, 18, 19 }, { 20, 21, 22, 23, 24, 25, 26, 27, 28 } };
	auto serialized(type::make_serialize<type::interleaved_std140>(
		type::make_supplier(std::ref(array1)),
		type::make_supplier(std::ref(array2)),
		type::make_supplier(std::ref(array3))));
	const std::size_t size(5 * 4 * 2);
	ASSERT_EQ(type::size(serialized), sizeof(float) * size);
	float output[size];
	type::flush(serialized, output);
	ASSERT_EQ(output[0], 1);
	ASSERT_EQ(output[1], 2);
	ASSERT_EQ(output[2], 3);
	ASSERT_EQ(output[4], 7);
	ASSERT_EQ(output[5], 8);
	ASSERT_EQ(output[8], 11);
	ASSERT_EQ(output[9], 12);
	ASSERT_EQ(output[10], 13);
	ASSERT_EQ(output[12], 14);
	ASSERT_EQ(output[13], 15);
	ASSERT_EQ(output[14], 16);
	ASSERT_EQ(output[16], 17);
	ASSERT_EQ(output[17], 18);
	ASSERT_EQ(output[18], 19);
	ASSERT_EQ(output[20], 4);
	ASSERT_EQ(output[21], 5);
	ASSERT_EQ(output[22], 6);
	ASSERT_EQ(output[24], 9);
	ASSERT_EQ(output[25], 10);
	ASSERT_EQ(output[28], 20);
	ASSERT_EQ(output[29], 21);
	ASSERT_EQ(output[30], 22);
	ASSERT_EQ(output[32], 23);
	ASSERT_EQ(output[33], 24);
	ASSERT_EQ(output[34], 25);
	ASSERT_EQ(output[36], 26);
	ASSERT_EQ(output[37], 27);
	ASSERT_EQ(output[38], 28);
}

TEST(SerializeTypeTest, GroupedInterleavedStd140ArrayLayout1) {
	type::t_array<float> array1({ 1, 2 });
	type::t_array<glm::vec2> array2{{ {1, 2}, {2, 3}, {3, 4} }};
	type::t_array<glm::vec3> array3{{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} }};
	type::t_array<glm::vec3> array4{{ {10, 11, 12}, {13, 14, 15}, {16, 17, 18}, {19, 20, 21} }};
	auto serialized(type::make_serialize<type::interleaved_std140>(
			type::make_supplier(std::ref(array1)),
			type::make_supplier(std::ref(array2)),
			type::make_supplier(std::ref(array3)),
			type::make_supplier(std::ref(array4))));
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
