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
#ifndef GTYPE_MEMORY_H_
#define GTYPE_MEMORY_H_

#include <cstddef>

namespace type {

enum memory_layout {
	// With linear, the data given to create will be concatenated after each other tightly.
	// It is up to the developer to make sure they are aligned nicely, for example on a vec4 boundary.
	linear = 0,
	// With interleaved_std140 all elements are interleaved to be used as vertex input.
	// They are also aligned with a vec4 boundary, i.e. all vec3 will be padded to vec4.
	// NOTICE: arrays of length 1 are treated as single elements, not arrays, which are not necessarily padded.
	// NOTICE: Interleaving only works when arrays are the same length, therefore, the data will be interleaved in groups.
	//		   For example, N vertex, N normals, 4*N texcoord, 4*N color will result in:
	//		   vertex(0), normal(0), ..., vertex(N-1), normal(N-1), texcoord(0), color(0), ..., texcoord(4*N-1), color(4*N-1)
	// See: https://developer.apple.com/library/ios/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/TechniquesforWorkingwithVertexData/TechniquesforWorkingwithVertexData.html
	//		"Use Interleaved Vertex Data" specifically "An exception to this rule..."
	interleaved_std140,
	// Same as linear but alignment according to std140
	linear_std140,
	interleaved_std430,
	linear_std430,
	// TODO(gardell): Possibly support other types too.
	num_layouts
};

}  // namespace type

#endif // GTYPE_MEMORY_H_
