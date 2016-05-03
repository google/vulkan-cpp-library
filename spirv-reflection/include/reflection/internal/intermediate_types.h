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
#ifndef SPIRV_REFLECTION_INTERMEDIATE_TYPES_H_
#define SPIRV_REFLECTION_INTERMEDIATE_TYPES_H_

#include <reflection/internal/includes.h>
#include <reflection/types.h>
#include <unordered_map>
#include <vector>

namespace spirv {
namespace internal {

struct constant_type {
	uint32_t result_type, result_id;
	std::vector<uint32_t> value;
};

struct variable_type {
	uint32_t result_type_id, result_id;
	SpvStorageClass storage_class;
};

struct name_type {
	std::string name;
	uint32_t type_id;
};

struct member_names_type {
	std::unordered_map<uint32_t, std::string> names;
};

struct member_offsets_type {
	std::unordered_map<uint32_t, uint32_t> offsets;
};

struct type_pointer_type {
	uint32_t result_id;
	SpvStorageClass storage_class;
	uint32_t type_id;
};

struct decoration_type {
	uint32_t target_id;
	SpvDecoration decoration;
	uint32_t operand; // optional
};

struct linkage_decoration_type {
	uint32_t target_id;
	SpvLinkageType linkage_type;
	std::string name;
};

// TODO: Try to merge with type declaration to use same logic
struct decoration_group_type {
	uint32_t result_id;
};

struct group_decorate_type {
	std::vector<uint32_t> target_ids;
	uint32_t decoration_group_id;
};

struct group_member_decorate_type {
	std::vector<std::pair<uint32_t, uint32_t>> target_ids;
	uint32_t decoration_group_id;
};

// Includes void, bool, float, vector, matrix, array, runtime array.
struct primitive_type {
	SpvOp op;
	uint32_t result_id, type, arg;
};

struct image_type {
	uint32_t result_id, sampled_id;
	SpvDim dim;
	uint32_t depth;
	bool arrayed;
	bool multisampled;
	uint32_t sampled;
	SpvImageFormat format;
};

struct struct_type {
	uint32_t result_id;
	std::vector<identifier_type> member_ids;
};

// TODO(gardell): We don't care about OpConstantSampler.
struct constant_sampler_type {
	uint32_t result_id;
	SpvSamplerAddressingMode addressing_mode;
	bool normalized;
	SpvSamplerFilterMode filter_mode;
};

struct sampler_type {
	uint32_t result_id;
};

struct sampled_image_type {
	uint32_t result_id, image_id;
};

struct sampled_image_binding_type {
	uint32_t result_type, result_id, image_id, sampler_id;
};

struct intermediate_type {
	std::unordered_map<uint32_t, constant_type> constants;
	std::unordered_map<uint32_t, variable_type> variables;
	std::unordered_map<uint32_t, name_type> names;
	std::unordered_map<uint32_t, member_names_type> member_names;
	std::unordered_map<uint32_t, member_offsets_type> member_offsets;
	std::unordered_map<uint32_t, type_pointer_type> type_pointers;
	std::unordered_map<uint32_t, std::vector<decoration_type>> decorations;
	std::unordered_map<uint32_t, linkage_decoration_type> linkage_decorations;
	std::unordered_map<uint32_t, decoration_group_type> decoration_groups;
	std::unordered_map<uint32_t, group_decorate_type> group_decorations;
	std::unordered_map<uint32_t, group_member_decorate_type> group_member_decorations;
	std::vector<entry_point_type> entry_points;
	std::unordered_map<uint32_t, primitive_type> primitives;
	std::unordered_map<uint32_t, image_type> images;
	std::unordered_map<uint32_t, struct_type> structs;
	std::unordered_map<uint32_t, sampler_type> samplers;
	std::unordered_map<uint32_t, constant_sampler_type> constant_samplers;
	std::unordered_map<uint32_t, sampled_image_type> sampled_images;
	std::unordered_map<uint32_t, sampled_image_binding_type> sampled_images_binding;
};

}  // namespace internal
}  // namespace spirv


#endif // SPIRV_REFLECTION_INTERMEDIATE_TYPES_H_
