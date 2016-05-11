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

TEST(SpirvAnalyzer, Subpass1) {
	const spirv::module_type module(spirv::parse(
		std::ifstream("../../subpass_test1.spv", std::ios_base::binary)));
	ASSERT_EQ(module.variables.size(), 1);
	const spirv::variable_type &variable(module.variables.begin()->second);
	ASSERT_EQ(variable.storage_class, SpvStorageClassUniformConstant);
	ASSERT_EQ(variable.name, "t");
	ASSERT_EQ(variable.binding, 0); // subpass inputs do not have a binding
	ASSERT_EQ(variable.location, 0); // subpass inputs do not have a location
	ASSERT_EQ(variable.descriptor_set, 1);
	ASSERT_EQ(variable.input_attachment_index, 2);

	const spirv::image_type &image(module.images.at(variable.type_id));
	ASSERT_EQ(image.dim, SpvDimSubpassData);
	ASSERT_FALSE(image.arrayed);
	ASSERT_FALSE(image.multisampled);
	const spirv::primitive_type &sampled(module.primitive_types.at(image.sampled_id));
	ASSERT_EQ(sampled.type, SpvOpTypeFloat);
	ASSERT_EQ(sampled.components[0], 1);
	ASSERT_EQ(sampled.components[1], 1);
	ASSERT_EQ(sampled.bits, 32);
	ASSERT_FALSE(sampled.array);
	ASSERT_FALSE(sampled.signedness);
	ASSERT_EQ(sampled.count_id, 0);
}
