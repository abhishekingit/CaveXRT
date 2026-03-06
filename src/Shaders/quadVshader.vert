#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;

out qVS_out {
	vec2 vUV;
	vec3 WorldPos;
	vec3 WorldNormal;
	vec3 FragPos;
	vec4 FragPosLightSpace;
} qvs_out;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;


void main() {
	qvs_out.vUV = aUV;
	qvs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	qvs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
	qvs_out.WorldNormal = mat3(transpose(inverse(model))) * vec3(0.0, 0.0, 1.0);
	qvs_out.FragPosLightSpace = lightSpaceMatrix * vec4(qvs_out.WorldPos, 1.0);
	gl_Position = mvp * vec4(aPos, 1.0);
}