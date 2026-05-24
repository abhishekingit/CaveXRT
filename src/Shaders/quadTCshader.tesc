#version 410 core

layout(vertices = 4) out;

in VS_OUT {
	vec3 localPos;
	vec2 uv;

} tcs_in[];

out TCS_OUT {
	vec3 localPos;
	vec2 uv;
} tcs_out[];

uniform float tessOuterLevel;
uniform float tessInnerLevel;

void main()
{
	tcs_out[gl_InvocationID].localPos = tcs_in[gl_InvocationID].localPos;
	tcs_out[gl_InvocationID].uv = tcs_in[gl_InvocationID].uv;

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	if(gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = tessOuterLevel;
		gl_TessLevelOuter[1] = tessOuterLevel;
		gl_TessLevelOuter[2] = tessOuterLevel;
		gl_TessLevelOuter[3] = tessOuterLevel;

		gl_TessLevelInner[0] = tessInnerLevel;
		gl_TessLevelInner[1] = tessInnerLevel;
	}

}