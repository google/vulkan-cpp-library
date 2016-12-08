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

const spirv::variable_type &find_variable_by_location(const spirv::module_type &module,
		spirv::identifier_type location) {
	for (const auto &variable : module.variables) {
		if (variable.second.location == location) {
			return variable.second;
		}
	}
	throw std::logic_error("Could not find variable by location");
}

TEST(SpirvAnalyzer, Input1) {
	const spirv::module_type module(spirv::parse(
		std::ifstream("input_test1.spv", std::ios_base::binary)));
	ASSERT_EQ(module.variables.size(), 3);
	const spirv::variable_type &variable1(find_variable_by_location(module, 1));
	ASSERT_EQ(variable1.storage_class, SpvStorageClassInput);
	ASSERT_EQ(variable1.name, "vertex");
	ASSERT_EQ(variable1.binding, 0);
	ASSERT_EQ(variable1.location, 1);
	ASSERT_EQ(variable1.descriptor_set, 0);
	ASSERT_EQ(variable1.input_attachment_index, 0);
	ASSERT_EQ(variable1.constant_id, 0);

	const spirv::variable_type &variable2(find_variable_by_location(module, 2));
	ASSERT_EQ(variable2.storage_class, SpvStorageClassInput);
	ASSERT_EQ(variable2.name, "model_view");
	ASSERT_EQ(variable2.binding, 0);
	ASSERT_EQ(variable2.location, 2);
	ASSERT_EQ(variable2.descriptor_set, 0);
	ASSERT_EQ(variable2.input_attachment_index, 0);
	ASSERT_EQ(variable2.constant_id, 0);

	const spirv::variable_type &variable3(find_variable_by_location(module, 5));
	ASSERT_EQ(variable3.storage_class, SpvStorageClassInput);
	ASSERT_EQ(variable3.name, "projection");
	ASSERT_EQ(variable3.binding, 0);
	ASSERT_EQ(variable3.location, 5);
	ASSERT_EQ(variable3.descriptor_set, 0);
	ASSERT_EQ(variable3.input_attachment_index, 0);
	ASSERT_EQ(variable3.constant_id, 0);
}
