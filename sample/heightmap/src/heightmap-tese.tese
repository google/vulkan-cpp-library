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
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(std140, binding = 0) uniform buf {
		mat4 projection_matrix;
		mat4 modelview_matrix;
} ubuf;

layout (binding = 2) uniform sampler2D height;

layout(quads, equal_spacing, cw) in;

layout (location = 0) in vec2 tese_vertex[];

layout (location = 0) out vec2 texcoord;
layout (location = 1) out float ec_z;

out gl_PerVertex {
        vec4 gl_Position;
};

const float height_scaling = 1.0 / 128;
const float y_scaling = 32;

void main() {
   vec2 vertex = gl_TessCoord.xy + tese_vertex[0];
   texcoord = gl_TessCoord.xy;

   float y = y_scaling * texture(height, height_scaling * vertex).x;
   vec4 ec_vertex = ubuf.modelview_matrix * vec4(vertex.x, y, vertex.y, 1.0);
   ec_z = length(ec_vertex);
   gl_Position = ubuf.projection_matrix * ec_vertex;
}
