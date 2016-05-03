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
#ifndef SPIRV_REFLECTION_ARGUMENT_PARSER_H_
#define SPIRV_REFLECTION_ARGUMENT_PARSER_H_

#include <reflection/internal/includes.h>
#include <string>
#include <vector>

namespace spirv {
namespace internal {

struct instruction_parser {

	explicit instruction_parser(const spv_parsed_instruction_t &instruction)
		: instruction(instruction) {}

	template<typename T>
	T single(std::size_t operand) const {
		return T(instruction.words[instruction.operands[operand].offset]);
	}

	bool single_bool(std::size_t operand) const {
		return !!instruction.words[instruction.operands[operand].offset];
	}

	template<typename T>
	T optional(std::size_t operand, T default_value) const {
		return operand >= instruction.num_operands ? default_value : single<T>(operand);
	}

	template<typename T>
	std::vector<T> multi(std::size_t operand) const {
		const T *words((T *) &instruction.words[instruction.operands[operand].offset]);
		return std::vector<T>(words,
			words + instruction.operands[operand].num_words * sizeof(uint32_t) / sizeof(T));
	}

	// Ignores num_words and just converts the "rest" of available arguments
	// Some commands will return num_words=1 for every argument even though its an array (2d-array effect/non-int type)
	template<typename T>
	std::vector<T> rest(std::size_t operand) const {
		if (operand < instruction.num_operands) {
			const T *words((T *)&instruction.words[instruction.operands[operand].offset]);
			const std::size_t count(instruction.words + instruction.num_words - words);
			return std::vector<T>(words, words + count * sizeof(uint32_t) / sizeof(T));
		}
		else {
			return std::vector<T>();
		}
	}

	std::string string(std::size_t operand) const {
		const char *words((const char *) &instruction.words[instruction.operands[operand].offset]);
		return std::string(words);
	}

	const spv_parsed_instruction_t &instruction;
};

}  // namespace internal
}  // namespace spirv

#endif // SPIRV_REFLECTION_ARGUMENT_PARSER_H_
