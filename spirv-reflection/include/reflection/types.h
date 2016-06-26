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
#ifndef SPIRV_REFLECTION_DATA_H_
#define SPIRV_REFLECTION_DATA_H_

#include <reflection/internal/includes.h>
#include <unordered_map>
#include <vector>

namespace spirv {

typedef uint32_t identifier_type;

struct constant_type {
	identifier_type id, type_id;
	std::vector<uint32_t> value; // might be larger than actual size. check primitive for bits.
	std::string name;
	bool specialization;
	identifier_type specialization_id;
};

struct member_type {
	identifier_type type_id;
	std::string name;
	uint32_t offset;
};

struct struct_type {
	bool array;
	identifier_type count_id;
	std::string name;
	std::vector<member_type> members;
};

struct primitive_type {
	SpvOp type; // float, int, bool, (only primitive single types).
	unsigned int components[2]; // [1..4]
	unsigned int bits; // 16, 32, 64
	bool array, signedness;
	identifier_type count_id;
};

struct image_type {
	identifier_type result_id, sampled_id;
	SpvDim dim;
	bool arrayed;
	bool multisampled;
};

// TODO(gardell): Possibly rename since its used with VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
struct sampled_image_type {
	identifier_type sampler_id, image_id; // sampler_id is OpTypeSampler, image_id is OpTypeImage.
};

struct sampler_type {
	identifier_type sampler;
};

struct variable_type {
	SpvStorageClass storage_class; // uniform, input etc.
	identifier_type identifier;
	std::string name;
	identifier_type type_id;
	identifier_type binding, location, descriptor_set, input_attachment_index, constant_id;
};

struct entry_point_type {
	SpvExecutionModel execution_model;
	identifier_type function_id;
	std::string name;
	std::vector<identifier_type> target_ids;
};

}  // namespace spirv

#endif // SPIRV_REFLECTION_DATA_H_
