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

TEST(SpirvAnalyzer, PushConstant1) {
	spirv::module_type module(spirv::parse(
			std::ifstream("push_constant_test1.spv", std::ios_base::binary)));
	ASSERT_EQ(module.variables.size(), 1);
	const spirv::variable_type &variable(module.variables.begin()->second);
	ASSERT_EQ(variable.storage_class, SpvStorageClassPushConstant);
	ASSERT_EQ(variable.name, "instance_name");
	ASSERT_EQ(variable.binding, 0); // push_constants do not have bindings.
	ASSERT_EQ(variable.location, 0); // push_constants do not have locations.
	ASSERT_EQ(variable.input_attachment_index, 0); // push_constants do not have
												   // input_attachment_index.
	ASSERT_EQ(variable.descriptor_set, 0); // push_constants do not have a descriptor_set.
	ASSERT_TRUE(module.constant_types.empty());
	ASSERT_TRUE(module.images.empty());
	ASSERT_TRUE(module.samplers.empty());
	ASSERT_TRUE(module.sampled_images.empty());
	ASSERT_EQ(module.entry_points.size(), 1);
	ASSERT_EQ(module.entry_points.size(), 1);
	ASSERT_EQ(module.struct_types.size(), 1);
	const spirv::struct_type &struct_(module.struct_types.begin()->second);
	ASSERT_FALSE(struct_.array);
	ASSERT_EQ(struct_.count_id, 0);
	ASSERT_EQ(struct_.members.size(), 3);
	ASSERT_EQ(struct_.name, "block_name");

	const spirv::member_type &member1(struct_.members.at(0));
	const spirv::primitive_type &primitive_member1(
		module.primitive_types.at(member1.type_id));
	ASSERT_EQ(primitive_member1.type, SpvOpTypeInt);
	ASSERT_EQ(primitive_member1.components[0], 1);
	ASSERT_EQ(primitive_member1.components[1], 1);
	ASSERT_EQ(primitive_member1.bits, 32);
	ASSERT_FALSE(primitive_member1.array);
	ASSERT_TRUE(primitive_member1.signedness);
	ASSERT_EQ(primitive_member1.count_id, 0);
	ASSERT_EQ(member1.name, "member1");

	const spirv::member_type &member2(struct_.members.at(1));
	const spirv::primitive_type &primitive_member2(
		module.primitive_types.at(member2.type_id));
	ASSERT_EQ(primitive_member2.type, SpvOpTypeFloat);
	ASSERT_EQ(primitive_member2.components[0], 1);
	ASSERT_EQ(primitive_member2.components[1], 1);
	ASSERT_EQ(primitive_member2.bits, 32);
	ASSERT_FALSE(primitive_member2.array);
	ASSERT_FALSE(primitive_member2.signedness);
	ASSERT_EQ(primitive_member2.count_id, 0);
	ASSERT_EQ(member2.name, "member2");

	const spirv::member_type &member3(struct_.members.at(2));
	const spirv::primitive_type &primitive_member3(
		module.primitive_types.at(member3.type_id));
	ASSERT_EQ(primitive_member3.type, SpvOpTypeFloat);
	ASSERT_EQ(primitive_member3.components[0], 3);
	ASSERT_EQ(primitive_member3.components[1], 1);
	ASSERT_EQ(primitive_member3.bits, 32);
	ASSERT_FALSE(primitive_member3.array);
	ASSERT_FALSE(primitive_member3.signedness);
	ASSERT_EQ(primitive_member3.count_id, 0);
	ASSERT_EQ(member3.name, "member3");
}
