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
#ifndef SPIRV_ANALYZER_ANALYZER_H_
#define SPIRV_ANALYZER_ANALYZER_H_

#include <reflection/types.h>

namespace spirv {

template<typename T>
using map_type = std::unordered_map<identifier_type, T>;

struct module_type {
	map_type<constant_type> constant_types;
	map_type<struct_type> struct_types;
	map_type<primitive_type> primitive_types;
	map_type<variable_type> variables;
	map_type<image_type> images; // TODO(gardell): Name image_types
	map_type<sampler_type> samplers; // sampler_types;
	map_type<sampled_image_type> sampled_images; // sampled_image_types
	std::vector<entry_point_type> entry_points;
};

namespace internal {

struct intermediate_type;

module_type convert(const intermediate_type &intermediate);

intermediate_type parse_intermediate(std::istream &stream);

module_type parse(std::istream &stream);

}  // namespace internal

template<typename StreamT>
module_type parse(StreamT &&stream) {
	return internal::parse(stream);
}

/**
 * Get all the variables references by a given function, this includes searching through
 * variables references by intermediate functions.
 */
std::vector<variable_type> variable_references(const module_type &module, const std::string &name);

}  // namespace spirv

#endif // SPIRV_REFLECTION_ANALYZER_H_
