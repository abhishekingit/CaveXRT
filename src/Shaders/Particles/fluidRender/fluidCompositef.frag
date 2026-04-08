#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D fluidDepthTexture;
uniform sampler2D fluidThicknessTexture;
uniform sampler2D fluidNormalTexture;
uniform samplerCube skybox;

uniform vec2 texelSize;
uniform mat4 projection;
uniform mat4 inverseView;
uniform float absorption;
uniform float refractionStrength;
uniform float specularIntensity;
uniform float shininess;
uniform float fresnelPower;
uniform float planeReflectionStrength;
uniform vec3 lightDirView;
uniform vec3 cameraPosWorld;
uniform float planeY;
uniform float planeHalfExtent;
uniform vec3 shallowColor;
uniform vec3 deepColor;

vec3 desaturate(vec3 color, float amount) {
	float l = dot(color, vec3(0.2126, 0.7152, 0.0722));
	return mix(color, vec3(l), amount);
}

vec3 samplePlaneColor(vec2 uv) {
	vec2 suv = fract(uv);
	vec3 pink = vec3(0.82, 0.67, 0.76);
	vec3 blue = vec3(0.62, 0.72, 0.86);
	vec3 yellow = vec3(0.86, 0.82, 0.60);
	vec3 green = vec3(0.64, 0.82, 0.66);

	vec3 quadrantColor;
	if (suv.x < 0.5 && suv.y < 0.5) quadrantColor = pink;
	else if (suv.x >= 0.5 && suv.y < 0.5) quadrantColor = blue;
	else if (suv.x < 0.5 && suv.y >= 0.5) quadrantColor = yellow;
	else quadrantColor = green;

	vec2 checkerUV = suv * 50.0;
	vec2 cell = floor(checkerUV);
	float checker = mod(cell.x + cell.y, 2.0);
	float shade = mix(0.90, 1.0, checker);

	vec3 base = desaturate(quadrantColor * shade, 0.55);
	return base * vec3(0.92, 0.96, 1.00);
}

vec3 reconstructViewPos(vec2 uv, float linearDepth) {
	vec2 ndc = uv * 2.0 - 1.0;
	float x = ndc.x * linearDepth / projection[0][0];
	float y = ndc.y * linearDepth / projection[1][1];
	return vec3(x, y, -linearDepth);
}

vec2 worldToPlaneUV(vec3 p) {
	float u = (p.x / (2.0 * planeHalfExtent)) + 0.5;
	float v = (-p.z / (2.0 * planeHalfExtent)) + 0.5;
	return vec2(u, v);
}

void main() {
	float depthVal = texture(fluidDepthTexture, vUV).r;
	float thickness = texture(fluidThicknessTexture, vUV).r;

	if(thickness <= 1e-6 || depthVal <= 0.0 ) {
		discard;
	}

	vec3 N = texture(fluidNormalTexture, vUV).xyz * 2.0 - 1.0;
	N = normalize(N);

	vec3 viewPos = reconstructViewPos(vUV, depthVal);
	vec3 worldPos = (inverseView * vec4(viewPos, 1.0)).xyz;
	vec3 Nw = normalize(mat3(inverseView) * N);
	vec3 V = normalize(-viewPos);
	vec3 L = normalize(lightDirView);
	vec3 H = normalize(L + V);

	float NDotL = max(dot(N, L), 0.0);
	float spec = pow(max(dot(N, H), 0.0), shininess) * specularIntensity;

	float depthAtten = exp(-thickness * absorption);
	vec3 bodyColor = mix(deepColor, shallowColor, depthAtten);

	vec3 Iw = normalize(worldPos - cameraPosWorld);
	vec3 reflDirW = reflect(Iw, Nw);
	vec3 refrDirW = refract(Iw, Nw, 1.0 / 1.333);
	if (length(refrDirW) < 1e-5) {
		refrDirW = Iw;
	}
	refrDirW = normalize(refrDirW);
	refrDirW = normalize(mix(refrDirW, Iw, clamp(1.0 - refractionStrength, 0.0, 1.0)));

	vec3 skyReflDir = normalize(vec3(reflDirW.x, abs(reflDirW.y), reflDirW.z));
	vec3 reflectionColor = texture(skybox, skyReflDir).rgb;
	reflectionColor = mix(reflectionColor, vec3(dot(reflectionColor, vec3(0.2126, 0.7152, 0.0722))), 0.30);
	reflectionColor *= 0.62;
	vec3 refractionColor = reflectionColor;

	float tRefract = (planeY - worldPos.y) / max(abs(refrDirW.y), 1e-4);
	if (refrDirW.y > -1e-4) tRefract = 0.0;
	vec3 refractHit = worldPos + refrDirW * tRefract;
	vec2 refractUV = worldToPlaneUV(refractHit);

	vec3 planeRefractColor = samplePlaneColor(refractUV);
	planeRefractColor = mix(planeRefractColor, planeRefractColor * vec3(0.88, 0.95, 1.02), 0.12);

	float tReflect = (planeY - worldPos.y) / max(abs(reflDirW.y), 1e-4);
	if (reflDirW.y > -1e-4) tReflect = 0.0;
	vec3 reflectHit = worldPos + reflDirW * tReflect;
	vec2 reflectUV = worldToPlaneUV(reflectHit);
	vec3 planeReflectColor = samplePlaneColor(reflectUV);
	planeReflectColor = mix(planeReflectColor, planeReflectColor * vec3(0.92, 0.96, 1.00), 0.08);

	vec3 planeReflectionColor = planeReflectColor;

	refractionColor = mix(planeRefractColor, refractionColor, 0.15);
	float planarReflectionWeight = clamp(planeReflectionStrength * 0.45, 0.0, 1.0);
	reflectionColor = mix(reflectionColor, planeReflectionColor, planarReflectionWeight);

	float fresnel = pow(1.0 - max(dot(N, V), 0.0), fresnelPower);
	vec3 refractBlue = refractionColor * mix(vec3(1.0), bodyColor, 0.45);
	float reflectionMix = clamp(0.02 + 0.30 * fresnel, 0.0, 0.55);
	vec3 envColor = mix(refractBlue, reflectionColor, reflectionMix);
	vec3 lit = envColor * (0.22 + 0.78 * NDotL) + vec3(spec);
	lit = mix(lit, lit * vec3(0.75, 0.85, 1.02), 0.20);

	float thicknessMask = clamp(thickness * 0.12, 0.0, 1.0);
	float edgeMask = 1.0 - thicknessMask;

	vec3 edgeColor = mix(lit, refractBlue, edgeMask * 0.06);
	float edgeLuma = dot(edgeColor, vec3(0.2126, 0.7152, 0.0722));
	edgeColor = mix(edgeColor, vec3(edgeLuma), edgeMask * 0.05);

	float alpha = clamp(0.60 + thickness * 0.60, 0.38, 0.96);
	vec3 finalColor = edgeColor;
	FragColor = vec4(finalColor, alpha);
}