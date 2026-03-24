#version 410 core

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

uniform bool useDisplacementMap;
uniform sampler2D displacementMap;
uniform float displacementScale;


void main() {
	qvs_out.vUV = aUV;
	vec3 localPos = aPos;
	if(useDisplacementMap) {
		float h = texture(displacementMap, aUV).r;
		float disp = (h * 2.0 - 1.0) * displacementScale;
		localPos += vec3(0.0, 0.0, 1.0) * disp;

	}

	vec4 world = model * vec4(localPos, 1.0);
	qvs_out.FragPos = world.xyz;
	qvs_out.WorldPos = world.xyz;
	qvs_out.WorldNormal = mat3(transpose(inverse(model))) * vec3(0.0, 0.0, 1.0);
	qvs_out.FragPosLightSpace = lightSpaceMatrix * world;
	gl_Position = mvp * vec4(localPos, 1.0);
}