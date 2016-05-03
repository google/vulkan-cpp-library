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
#include <fstream>
#include <gtest/gtest.h>
#include <reflection/analyzer.h>

TEST(SpirvAnalyzer, SpecializationConstant1) {
	const spirv::module_type module(spirv::parse(
		std::ifstream("specialization_constant_test1.spv", std::ios_base::binary)));
	ASSERT_TRUE(module.variables.empty());
	ASSERT_EQ(module.constant_types.size(), 1);
	const spirv::constant_type &constant(module.constant_types.begin()->second);
	ASSERT_EQ(constant.value.size(), 1);
	ASSERT_EQ(constant.value[0], 12);
	// TODO(gardell): compiler doesn't export name of specialization constants.
	//ASSERT_EQ(constant.name, "value");
	ASSERT_TRUE(constant.specialization);
	ASSERT_EQ(constant.specialization_id, 1);

	const spirv::primitive_type &primitive(module.primitive_types.at(constant.type_id));
	ASSERT_EQ(primitive.type, SpvOpTypeInt);
	ASSERT_EQ(primitive.components[0], 1);
	ASSERT_EQ(primitive.components[1], 1);
	ASSERT_EQ(primitive.bits, 32);
	ASSERT_EQ(primitive.array, false);
	ASSERT_EQ(primitive.signedness, true);
	ASSERT_EQ(primitive.count_id, 0);
}
