#version 410 core

layout(quads, equal_spacing, ccw) in;

in TCS_OUT {
	vec3 localPos;
	vec2 uv;

} tes_in[];

out qVS_out {
	vec2 vUV;
	vec3 WorldPos;
	vec3 WorldNormal;
	vec3 FragPos;
	vec4 FragPosLightSpace;

 } tes_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform bool useDisplacementMap;
uniform sampler2D displacementMap;
uniform float displacementScale;

uniform bool useDispShadows;

void main() 
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec3 p0 = mix(tes_in[0].localPos, tes_in[1].localPos, u);
	vec3 p1 = mix(tes_in[3].localPos, tes_in[2].localPos, u);
	vec3 localPos = mix(p0, p1, v);

	vec2 uv0 = mix(tes_in[0].uv, tes_in[1].uv, u);
	vec2 uv1 = mix(tes_in[3].uv, tes_in[2].uv, u);
	vec2 uv = mix(uv0, uv1, v);

	if(useDisplacementMap) {
		float h = texture(displacementMap, uv).r;
		float d = (h * 2.0 - 1.0) * displacementScale;
		localPos += vec3(0.0, 0.0, 1.0) * d;
	}

	vec4 world = model * vec4(localPos, 1.0);

	tes_out.vUV = uv;
	tes_out.WorldPos = world.xyz;
	tes_out.WorldNormal = normalize(mat3(transpose(inverse(model))) * vec3(0.0, 0.0, 1.0));
	tes_out.FragPos = world.xyz;
	tes_out.FragPosLightSpace = lightSpaceMatrix * world;

	if(useDispShadows) {
		gl_Position = lightSpaceMatrix * world;
	}
	else {
		gl_Position = projection * view * world;
	}
	

}
