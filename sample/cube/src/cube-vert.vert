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
		mat4 modelview_projection_matrix;
} ubuf;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 in_texcoord;

layout (location = 0) out vec2 texcoord;

out gl_PerVertex {
        vec4 gl_Position;
};

void main() {
   texcoord = in_texcoord;
   gl_Position = ubuf.modelview_projection_matrix * vec4(vertex, 1.0);

   // GL->VK conventions
   gl_Position.y = -gl_Position.y;
   gl_Position.z = gl_Position.z / 2.0;
}
