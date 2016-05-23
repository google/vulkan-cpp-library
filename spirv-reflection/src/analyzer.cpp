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
#include <reflection/analyzer.h>
#include <reflection/internal/argument_parser.h>
#include <reflection/internal/intermediate_types.h>
#include <cassert>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <iostream>

// TODO(gardell): ConstantComposites are not supported
// TODO(gardell): Endianess not supported

namespace spirv {
namespace internal {

spv_result_t parsed_header(void* user_data, spv_endianness_t endian, uint32_t magic, uint32_t version,
		uint32_t generator, uint32_t id_bound, uint32_t reserved) {
#if 0
	std::cout << "endianness: " << (endian == SPV_ENDIANNESS_LITTLE
			? "SPV_ENDIANNESS_LITTLE" : "SPV_ENDIANNESS_BIG")
		<< ", magic: " << magic << ", version: " << version << ", generator: " << generator
		<< ", id_bound: " << id_bound << std::endl;
#endif
	// TODO(gardell): Check endianness, magic, version, etc
	return SPV_SUCCESS;
}

spv_result_t parsed_instruction(void* user_data,
		const spv_parsed_instruction_t* parsed_instruction) {
	intermediate_type &intermediate(*((intermediate_type *) user_data));
	const uint32_t result_id(parsed_instruction->result_id);
	const SpvOp opcode((SpvOp) parsed_instruction->opcode);
	const instruction_parser parser(*parsed_instruction);
	switch (opcode) {
	case SpvOpConstant:
	case SpvOpSpecConstant:
		intermediate.constants.emplace(result_id, constant_type{
			parser.single<uint32_t>(0), result_id, parser.rest<uint32_t>(2)});
		break;
	case SpvOpSpecConstantTrue:
	case SpvOpSpecConstantFalse:
		intermediate.constants.emplace(result_id, constant_type{
			parser.single<uint32_t>(0), result_id, {!!(opcode == SpvOpSpecConstantTrue)}});
		break;
	case SpvOpSpecConstantComposite:
		std::cerr << "OpSpecConstantComposite not supported yet!" << std::endl;
		break;
	case SpvOpVariable:
		intermediate.variables.emplace(result_id, variable_type{
			parser.single<uint32_t>(0), result_id, parser.single<SpvStorageClass>(2) });
		break;
	case SpvOpName: {
		auto target_id(parser.single<uint32_t>(0));
		intermediate.names.emplace(target_id, name_type{parser.string(1), target_id});
		} break;
	case SpvOpMemberName:
		intermediate.member_names[parser.single<uint32_t>(0)].names.emplace(
			parser.single<uint32_t>(1), parser.string(2));
		break;
	case SpvOpMemberDecorate:
		switch(parser.single<SpvDecoration>(2)) {
		case SpvDecorationOffset:
			intermediate.member_offsets[parser.single<uint32_t>(0)].offsets.emplace(
				parser.single<uint32_t>(1), parser.single<uint32_t>(3));
			break;
		default:
			break;
		}
		break;
	case SpvOpTypePointer:
		intermediate.type_pointers.emplace(result_id, type_pointer_type{ result_id,
			parser.single<SpvStorageClass>(1), parser.single<uint32_t>(2) });
		break;
	case SpvOpDecorate: {
		const auto target_id(parser.single<uint32_t>(0));
		const auto decoration(parser.single<SpvDecoration>(1));
		switch (decoration) {
		case SpvDecorationLinkageAttributes:
			intermediate.linkage_decorations.emplace(target_id, linkage_decoration_type{
				target_id, parser.single<SpvLinkageType>(3) });
			break;
		default:
			intermediate.decorations[target_id].push_back(decoration_type{target_id, decoration,
				parser.optional<uint32_t>(2, 0)});
			break;
		}
		} break;
	case SpvOpDecorationGroup:
		std::cerr << "OpDecorationGroup is not yet implemented!" << std::endl;
		intermediate.decoration_groups.emplace(result_id, decoration_group_type{ result_id });
		break;
	case SpvOpGroupDecorate: {
		const auto decoration_group(parser.single<uint32_t>(0));
		intermediate.group_decorations.emplace(decoration_group, group_decorate_type{
			parser.multi<uint32_t>(1), decoration_group});
	} break;
	case SpvOpGroupMemberDecorate: {
		const auto decoration_group(parser.single<uint32_t>(0));
		intermediate.group_member_decorations.emplace(decoration_group, group_member_decorate_type{
			parser.multi<std::pair<uint32_t, uint32_t>>(1), decoration_group});
	} break;
	case SpvOpEntryPoint:
		intermediate.entry_points.push_back(entry_point_type{
			parser.single<SpvExecutionModel>(0), parser.single<uint32_t>(1), parser.string(2),
			parser.rest<uint32_t>(3)});
		break;
	case SpvOpTypeVoid:
	case SpvOpTypeBool:
		intermediate.primitives.emplace(result_id, primitive_type{opcode, result_id});
		break;
	case SpvOpTypeInt:
	case SpvOpTypeVector:
	case SpvOpTypeMatrix:
	case SpvOpTypeArray:
		intermediate.primitives.emplace(result_id, primitive_type{opcode, result_id,
			parser.single<uint32_t>(1), parser.single<uint32_t>(2)});
		break;
	case SpvOpTypeFloat:
	case SpvOpTypeRuntimeArray:
		intermediate.primitives.emplace(result_id, primitive_type{opcode, result_id,
			parser.single<uint32_t>(1)});
		break;
	case SpvOpTypeImage:
		intermediate.images.emplace(result_id, image_type{ result_id, parser.single<uint32_t>(1),
			parser.single<SpvDim>(2), parser.single<uint32_t>(3), parser.single_bool(4),
			parser.single_bool(5), parser.single<uint32_t>(6), parser.single<SpvImageFormat>(7) });
		break;
	case SpvOpTypeStruct:
		intermediate.structs.emplace(result_id,
				struct_type{result_id, parser.rest<uint32_t>(1)});
		break;
	case SpvOpTypeSampler:
		intermediate.samplers.emplace(result_id, sampler_type{result_id});
		break;
	case SpvOpConstantSampler:
		intermediate.constant_samplers.emplace(result_id, constant_sampler_type{ result_id,
			parser.single<SpvSamplerAddressingMode>(1), parser.single_bool(2),
			parser.single<SpvSamplerFilterMode>(3)});
		break;
	case SpvOpTypeSampledImage:
		intermediate.sampled_images.emplace(result_id, sampled_image_type{ result_id,
			parser.single<uint32_t>(1) });
		break;
	case SpvOpSampledImage:
		intermediate.sampled_images_binding.emplace(result_id, sampled_image_binding_type{
			parser.single<uint32_t>(0), result_id, parser.single<uint32_t>(2),
			parser.single<uint32_t>(2) });
		break;
	default:
		break;
	}
	return SPV_SUCCESS;
}

spirv::primitive_type parse_primitive(const intermediate_type &intermediate,
		const primitive_type &primitive) {
	switch (primitive.op) {
	case SpvOpTypeVoid:
		return spirv::primitive_type{ primitive.op, { 0, 0 }, 0, false, false, 0 };
	case SpvOpTypeVector: {
		spirv::primitive_type result_primitive(parse_primitive(intermediate,
			intermediate.primitives.find(primitive.type)->second));
		result_primitive.components[0] = primitive.arg;
		result_primitive.components[1] = 1;
		return result_primitive;
	} break;
	case SpvOpTypeBool:
	case SpvOpTypeFloat:
	case SpvOpTypeInt:
		return spirv::primitive_type{primitive.op, { 1, 1 }, primitive.type, false,
			!!primitive.arg, 0 };
	case SpvOpTypeMatrix: {
		spirv::primitive_type result_primitive(parse_primitive(intermediate,
			intermediate.primitives.find(primitive.type)->second));
		result_primitive.components[1] = primitive.arg;
		return result_primitive;
	} break;
	case SpvOpTypeSampledImage:
		// TODO(gardell): Handle sampled images.
		return spirv::primitive_type{primitive.op, { 1 }, 0, false, 0, primitive.type };
	default:
		std::cout << primitive.op << std::endl;
		throw std::logic_error("Invalid primitive type");
	}
}

void parse_primitive(const primitive_type &primitive, const intermediate_type &intermediate,
		module_type &module) {
	switch (primitive.op) {
	case SpvOpTypeArray: {
		auto element_primitive_it(intermediate.primitives.find(primitive.type));
		if (element_primitive_it != intermediate.primitives.end()) {
			spirv::primitive_type element_primitive(parse_primitive(intermediate,
				element_primitive_it->second));
				element_primitive.array = true;
				element_primitive.count_id = primitive.arg;
				module.primitive_types.emplace(primitive.result_id, element_primitive);
		} else {
			auto struct_it(intermediate.structs.find(primitive.type));
			if (struct_it != intermediate.structs.end()) {
				spirv::struct_type struct_{true, primitive.arg};
				const auto struct_name_it(intermediate.names.find(struct_it->first));
				if (struct_name_it != intermediate.names.end()) {
					struct_.name = struct_name_it->second.name;
				}
				for (uint32_t i = 0; i < uint32_t(struct_it->second.member_ids.size()); ++i) {
					struct_.members.push_back(member_type{struct_it->second.member_ids[i],
						intermediate.member_names.at(struct_it->first).names.at(i),
						intermediate.member_offsets.at(struct_it->first).offsets.at(i)});
				}
				module.struct_types.emplace(primitive.result_id, std::move(struct_));
			}
		}
	} break;
	case SpvOpTypeSampledImage: {

	} break;
	default:
		module.primitive_types.emplace(primitive.result_id, parse_primitive(intermediate, primitive));
		break;
	}
}

void parse_type(identifier_type type_id, const intermediate_type &intermediate,
		module_type &module) {
	const auto struct_it(intermediate.structs.find(type_id));
	if (struct_it != intermediate.structs.end()) {
		spirv::struct_type struct_{false, 0};
		const auto name_it(intermediate.names.find(struct_it->first));
		if (name_it != intermediate.names.end()) {
			struct_.name = name_it->second.name;
		}
		for (uint32_t i = 0; i < uint32_t(struct_it->second.member_ids.size()); ++i) {
			struct_.members.push_back(member_type{struct_it->second.member_ids[i],
				intermediate.member_names.at(struct_it->first).names.at(i),
				intermediate.member_offsets.at(struct_it->first).offsets.at(i)});
		}
		module.struct_types.emplace(struct_it->first, struct_);
	} else {
		const auto primitive_it(intermediate.primitives.find(type_id));
		if (primitive_it != intermediate.primitives.end()) {
			const internal::primitive_type &primitive(primitive_it->second);
			parse_primitive(primitive, intermediate, module);
		}
	}
}

void parse_decoration(const std::vector<decoration_type> &decorations,
		spirv::variable_type &variable) {
	for (const decoration_type &decoration : decorations) {
		switch (decoration.decoration) {
		case SpvDecorationLocation:
			variable.location = decoration.operand;
			break;
		case SpvDecorationBinding:
			variable.binding = decoration.operand;
			break;
		case SpvDecorationInputAttachmentIndex:
			variable.input_attachment_index = decoration.operand;
			break;
		case SpvDecorationDescriptorSet:
			variable.descriptor_set = decoration.operand;
		default:
			break;
		}
	}
}

void parse_decoration(const std::vector<decoration_type> &decorations,
		spirv::constant_type &constant) {
	constant.specialization = false;
	for (const decoration_type &decoration : decorations) {
		switch (decoration.decoration) {
		case SpvDecorationSpecId:
		constant.specialization = true;
		constant.specialization_id = decoration.operand;
			break;
		default:
			break;
		}
	}
}

std::string get_name_or_empty(uint32_t id, const intermediate_type &context) {
	const auto name_it(context.names.find(id));
	return name_it != context.names.end() ? name_it->second.name : "";
}

module_type convert(const intermediate_type &intermediate) {
	module_type module;
	for (const std::pair<uint32_t, variable_type> &pair : intermediate.variables) {
		const auto type_pointer_it(intermediate.type_pointers.find(pair.second.result_type_id));
		const identifier_type type_id = type_pointer_it != intermediate.type_pointers.end()
			? type_pointer_it->second.type_id : pair.second.result_type_id;
		spirv::variable_type variable;
		variable.storage_class = pair.second.storage_class;
		variable.identifier = pair.first;
		variable.name = get_name_or_empty(pair.first, intermediate);
		variable.type_id = type_id;
		variable.binding = variable.location =
			variable.input_attachment_index = variable.descriptor_set =
			variable.constant_id = 0;

		const auto decorations_it(intermediate.decorations.find(pair.first));
		if (decorations_it != intermediate.decorations.end()) {
			parse_decoration(decorations_it->second, variable);
		}

		module.variables.emplace(variable.identifier, variable);
	}

	module.entry_points = intermediate.entry_points;

	for (const std::pair<uint32_t, constant_type> &pair : intermediate.constants) {
		spirv::constant_type constant{
			pair.first, pair.second.result_type, pair.second.value,
			get_name_or_empty(pair.first, intermediate)};
		const auto decorations_it(intermediate.decorations.find(pair.first));
		if (decorations_it != intermediate.decorations.end()) {
			parse_decoration(decorations_it->second, constant);
		}
		module.constant_types.emplace(pair.first, std::move(constant));
	}

	for (const std::pair<uint32_t, primitive_type> &pair : intermediate.primitives) {
		// TODO(gardell): Only call second branch in parse_type.
		parse_type(pair.first, intermediate, module);
	}

	for (const std::pair<uint32_t, type_pointer_type> &pair : intermediate.type_pointers) {
		parse_type(pair.second.type_id, intermediate, module);
	}

	for (const std::pair<uint32_t, struct_type> &pair : intermediate.structs) {
		for (identifier_type id : pair.second.member_ids) {
			parse_type(id, intermediate, module);
		}
	}

	for (const std::pair<uint32_t, image_type> &pair : intermediate.images) {
		module.images.emplace(pair.first, spirv::image_type{
			pair.second.result_id, pair.second.sampled_id, pair.second.dim, pair.second.arrayed,
			pair.second.multisampled });
	}

	for (const std::pair<uint32_t, sampler_type> &pair : intermediate.samplers) {
		module.samplers.emplace(pair.first,
			spirv::sampler_type{pair.second.result_id});
	}

	for (const std::pair<uint32_t, sampled_image_type> &pair : intermediate.sampled_images) {
		module.sampled_images.emplace(pair.first,
			spirv::sampled_image_type{pair.first, pair.second.image_id});
	}

	return std::move(module);
}

intermediate_type parse_intermediate(std::istream &stream) {
	std::shared_ptr<spv_context_t> context(
		spvContextCreate(SPV_ENV_VULKAN_1_0), spvContextDestroy);
	assert(context);

	const std::string content(std::istreambuf_iterator<char>(stream.rdbuf()),
			std::istreambuf_iterator<char>());
	intermediate_type intermediate;
	spv_diagnostic diagnostic;
	spvBinaryParse(context.get(), &intermediate, (uint32_t *) content.c_str(),
			content.size() / sizeof(uint32_t), &parsed_header, &parsed_instruction, &diagnostic);
	return intermediate;
}

module_type parse(std::istream &stream) {
	return convert(parse_intermediate(stream));
}

}  // namespace internal
}  // namespace spirv
