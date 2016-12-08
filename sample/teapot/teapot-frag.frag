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
layout (binding = 1) uniform samplerCube tex;

layout (location = 0) in vec2 texcoord;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 eye_position;
layout (location = 3) flat in int color_index;

layout (location = 0) out vec4 uFragColor;

layout(constant_id = 2) const int num_colors = 4;

layout(push_constant) uniform constants {
    vec4 colors[num_colors];
} constantsbuf;

layout(constant_id = 1) const int max_lights = 1;

struct light_type {
  vec4 position;
  vec3 attenuation;
  vec3 spot_direction;
  float spot_cos_cutoff;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float spot_exponent;
};

layout(std140, binding = 2) uniform lights {
        light_type lights[max_lights];
} lightsbuf;

void spotlight(in light_type light, in vec3 eye_position, in vec3 normal,
        inout vec4 ambient, inout vec4 diffuse, inout vec4 specular, in float shininess) { 
    float nDotVP;           // normal . light direction 
    float nDotHV;           // normal . light half vector 
    float pf;               // power factor 
    float spotDot;          // cosine of angle between spotlight 
    float spotAttenuation;  // spotlight attenuation factor 
    float attenuation;      // computed attenuation factor 
    float d;                // distance from surface to light source 
    vec3 VP;                // direction from surface to light position
    vec3 halfVector;        // direction of maximum highlights 
    // Compute vector from surface to light position 
    VP = vec3(light.position) - eye_position; 
    // Compute distance between surface and light position 
    d = length(VP); 
    // Normalize the vector from surface to light position 
    VP = normalize(VP); 
    // Compute attenuation 
    attenuation = 1.0 / (light.attenuation.x + 
                         light.attenuation.y * d + 
                         light.attenuation.z * d * d);

    if (light.spot_cos_cutoff != -1.0) {
        // See if point on surface is inside cone of illumination 
        spotDot = dot(-VP, normalize(light.spot_direction));
        
        if (spotDot < light.spot_cos_cutoff)
            spotAttenuation = 0.0; // light adds no contribution 
        else
            spotAttenuation = pow(spotDot, light.spot_exponent);
        
        // Combine the spotlight and distance attenuation. 
        attenuation *= spotAttenuation; 
    }

    halfVector = normalize(VP); 
    nDotVP = max(0.0, dot(normal, VP)); 
    nDotHV = max(0.0, dot(normal, halfVector)); 
    if (nDotVP == 0.0) 
        pf = 0.0; 
    else 
        pf = pow(nDotHV, shininess); 
    ambient  += light.ambient * attenuation; 
    diffuse  += light.diffuse * nDotVP * attenuation;
    specular += light.specular * pf * attenuation;
    diffuse = vec4(1.0);
}

vec3 reflection_map(in vec3 incident, in vec3 normal) {
  return reflect(incident, normal);
}

const float Eta = 0.66; // Ratio of indices of refraction
const float FresnelPower = 5.0;
const float F = ((1.0-Eta) * (1.0-Eta)) / ((1.0+Eta) * (1.0+Eta));

vec3 refraction_map(in vec3 incident, in vec3 normal) {
  float Ratio = F + (1.0 - F) * pow((1.0 - dot(-incident, normal)), FresnelPower);
  return refract(incident, normal, Eta);
}

float refraction_ratio(in vec3 incident, in vec3 normal) {
  return F + (1.0 - F) * pow((1.0 - dot(-incident, normal)), FresnelPower);
}

void main() {
  vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);
  //vec4 diffuse = texture(tex, texcoord);
  vec3 incident = normalize(eye_position);
  vec4 diffuse = texture(tex, mix(refraction_map(incident, normal), reflection_map(incident, normal), refraction_ratio(incident, normal)));
  //vec4 diffuse = texture(tex, reflection_map(incident, normal));
  vec4 specular = vec4(1.0, 1.0, 1.0, 1.0);
  float shininess = 2.0;

  vec4 amb = vec4(0.0), diff = vec4(0.0), spec = vec4(0.0);
  spotlight(lightsbuf.lights[0], eye_position, normal, amb, diff, spec, shininess);
  uFragColor = constantsbuf.colors[color_index % num_colors] * (amb * ambient + diff * diffuse + spec * specular);
}
