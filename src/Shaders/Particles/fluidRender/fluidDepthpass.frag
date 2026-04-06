#version 430 core

in vec3 ViewPos;
in float RadiusView;

layout(location = 0) out float FluidDepth;

uniform mat4 projection;

void main() {
	vec2 coord = (gl_PointCoord - vec2(0.5)) * 2.0;
	float r2 = dot(coord, coord);
	if(r2 > 1.0) {
		discard;
	}

	float z = sqrt(1.0 - r2);
	vec3 normalView = normalize(vec3(coord.x, coord.y, z));

	vec3 fragViewPos = ViewPos + normalView * RadiusView;

	vec4 clip = projection * vec4(fragViewPos, 1.0);
	float ndcDepth = clip.z / clip.w;
	float windowDepth = ndcDepth * 0.5 + 0.5;
	gl_FragDepth = windowDepth;

	FluidDepth = -fragViewPos.z;

}