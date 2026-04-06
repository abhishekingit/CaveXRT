#version 430 core

layout(location = 0) out float FluidThickness;

in float RadiusView;

void main() {
	vec2 coord = (gl_PointCoord - vec2(0.5)) * 2.0;
	float r2 = dot(coord, coord);
	if (r2 > 1.0) {
		discard;
	}

	float z = sqrt(1.0 - r2);

	FluidThickness = 2.0 * RadiusView * z;

}