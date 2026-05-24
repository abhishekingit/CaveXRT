#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;


out VS_OUT {
	vec3 localPos;
	vec2 uv;
} vs_out;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
	vs_out.localPos = aPos;
	vs_out.uv = aUV;
	gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}