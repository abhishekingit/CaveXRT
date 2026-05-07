#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D fluidDepthTexture;
uniform sampler2D fluidThicknessTexture;
uniform vec2 texelSize;
uniform mat4 projection;

vec3 reconstructViewPos(vec2 uv, float linearDepth) {
	vec2 ndc = uv * 2.0 - 1.0;
	float x = ndc.x * linearDepth / projection[0][0];
	float y = ndc.y * linearDepth / projection[1][1];
	return vec3(x, y, -linearDepth);
}

bool isFinite3(vec3 v) {
	return all(equal(v, v)) && all(lessThan(abs(v), vec3(1e19)));
}

void main() {
	float depthC = texture(fluidDepthTexture, vUV).r;
	float thickness = texture(fluidThicknessTexture, vUV).r;

	if (thickness <= 1e-6 || depthC <= 0.0) {
		FragColor = vec4(0.0);
		return;
	}

	vec2 uvL = vUV - vec2(texelSize.x, 0.0);
	vec2 uvR = vUV + vec2(texelSize.x, 0.0);
	vec2 uvD = vUV - vec2(0.0, texelSize.y);
	vec2 uvU = vUV + vec2(0.0, texelSize.y);

	float dL = texture(fluidDepthTexture, uvL).r;
	float dR = texture(fluidDepthTexture, uvR).r;
	float dD = texture(fluidDepthTexture, uvD).r;
	float dU = texture(fluidDepthTexture, uvU).r;

	if (dL <= 0.0) dL = depthC;
	if (dR <= 0.0) dR = depthC;
	if (dD <= 0.0) dD = depthC;
	if (dU <= 0.0) dU = depthC;

	vec3 pC = reconstructViewPos(vUV, depthC);
	vec3 pL = reconstructViewPos(uvL, dL);
	vec3 pR = reconstructViewPos(uvR, dR);
	vec3 pD = reconstructViewPos(uvD, dD);
	vec3 pU = reconstructViewPos(uvU, dU);

	vec3 dxl = pC - pL;
	vec3 dxr = pR - pC;
	vec3 dyb = pC - pD;
	vec3 dyt = pU - pC;

	vec3 dx = (abs(dxr.z) < abs(dxl.z)) ? dxr : dxl;
	vec3 dy = (abs(dyt.z) < abs(dyb.z)) ? dyt : dyb;

	vec3 nRaw = cross(dx, dy);
	float nLen2 = dot(nRaw, nRaw);
	vec3 N = (nLen2 > 1e-12) ? (nRaw * inversesqrt(nLen2)) : vec3(0.0, 0.0, -1.0);
	if (!isFinite3(N)) {
		N = vec3(0.0, 0.0, -1.0);
	}
	if (N.z > 0.0) {
		N = -N;
	}

	FragColor = vec4(N * 0.5 + 0.5, 1.0);
}
