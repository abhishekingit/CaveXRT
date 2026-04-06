#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D fluidDepthTexture;
uniform sampler2D fluidThicknessTexture;

void main() {
	float depthVal = texture(fluidDepthTexture, vUV).r;
	float thickness = texture(fluidThicknessTexture, vUV).r;

	if(thickness <= 1e-6 || depthVal <= 0.0 ) {
		discard;
	}

	float alpha = clamp(thickness * 25.0, 0.0, 0.95);
	vec3 deepColor = vec3(0.08, 0.18, 0.75);
	vec3 shallowColor = vec3(0.35, 0.65, 1.0);
	vec3 color = mix(deepColor, shallowColor, clamp(thickness * 0.05, 0.0, 1.0));
	FragColor = vec4(color, alpha);

	//FragColor = vec4(vec3(thickness, depthVal, 0.0), 1.0);
}