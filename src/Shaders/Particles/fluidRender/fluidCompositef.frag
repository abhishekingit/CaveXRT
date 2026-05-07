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
uniform vec3 absorption;
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
uniform float thicknessEpsilon;

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

vec3 safeNormalize(vec3 v) {
	float m2 = dot(v, v);
	return (m2 > 1e-8) ? v * inversesqrt(m2) : vec3(0.0, 0.0, 1.0);
}

void main() {
	float depthVal = texture(fluidDepthTexture, vUV).r;
	float thickness = texture(fluidThicknessTexture, vUV).r;
	float thicknessCutoff = max(thicknessEpsilon, 1e-5);

	if(thickness <= thicknessCutoff || depthVal <= 0.0 ) {
		discard;
	}

	vec3 N = texture(fluidNormalTexture, vUV).xyz * 2.0 - 1.0;
	N = safeNormalize(N);

	vec3 viewPos = reconstructViewPos(vUV, depthVal);
	vec3 worldPos = (inverseView * vec4(viewPos, 1.0)).xyz;
	vec3 Nw = safeNormalize(mat3(inverseView) * N);
	vec3 V = safeNormalize(-viewPos);
	vec3 L = safeNormalize(lightDirView);
	vec3 H = safeNormalize(L + V);

	float NDotL = max(dot(N, L), 0.0);
	float wrappedNDotL = clamp((dot(N, L) + 0.35) / 1.35, 0.0, 1.0);
	float spec = pow(max(dot(N, H), 0.0), max(8.0, shininess * 0.55)) * (specularIntensity * 1.10);

//	float depthAtten = exp(-thickness * absorption);
//	vec3 bodyColor = mix(deepColor, shallowColor, depthAtten);
	float thick = 1.0 - exp(-thickness * 0.35);
	vec3 transmittance = exp(-thick * absorption * 4.5);
	float thicknessNorm = clamp(1.0 - exp(-thickness * 0.10), 0.0, 1.0);
	float depthBlend = smoothstep(0.08, 0.85, thicknessNorm);
	vec3 bodyColor = mix(shallowColor, deepColor, depthBlend);

	vec3 Iw = safeNormalize(worldPos - cameraPosWorld);
	vec3 reflDirW = safeNormalize(reflect(Iw, Nw));
	vec3 refrDirW = refract(Iw, Nw, 1.0 / 1.333);
	if (length(refrDirW) < 1e-5) {
		refrDirW = Iw;
	}
	refrDirW = safeNormalize(refrDirW);
	refrDirW = safeNormalize(mix(refrDirW, Iw, clamp(1.0 - refractionStrength, 0.0, 1.0)));

	vec3 skyReflDir = safeNormalize(vec3(reflDirW.x, abs(reflDirW.y), reflDirW.z));
	vec3 reflectionColor = texture(skybox, skyReflDir).rgb;
	reflectionColor *= vec3(0.95, 1.00, 1.06);
	reflectionColor *= 0.90;
	vec3 refractionColor = reflectionColor;


	bool validRefract = (refrDirW.y < -1e-5);
	float tRefract = validRefract ? (planeY - worldPos.y) / refrDirW.y : 0.0;
	validRefract = validRefract && (tRefract > 0.0);
	vec3 refractHit = worldPos + refrDirW * tRefract;
	vec2 refractUV = clamp(worldToPlaneUV(refractHit), 0.0, 1.0);

	vec3 planeRefractColor = samplePlaneColor(refractUV);
	planeRefractColor = mix(planeRefractColor, planeRefractColor * vec3(0.88, 0.95, 1.02), 0.12);
	if (!validRefract) {
		planeRefractColor = mix(planeRefractColor, refractionColor, 0.75);
	}

	bool validReflect = (reflDirW.y < -1e-5);
	float tReflect = validReflect ? (planeY - worldPos.y) / reflDirW.y : 0.0;
	validReflect = validReflect && (tReflect > 0.0);
	vec3 reflectHit = worldPos + reflDirW * tReflect;
	vec2 reflectUV = clamp(worldToPlaneUV(reflectHit), 0.0, 1.0);
	vec3 planeReflectColor = samplePlaneColor(reflectUV);
	planeReflectColor = mix(planeReflectColor, planeReflectColor * vec3(0.92, 0.96, 1.00), 0.08);
	if (!validReflect) {
		planeReflectColor = mix(planeReflectColor, reflectionColor, 0.75);
	}

	vec3 planeReflectionColor = planeReflectColor;

	refractionColor = mix(planeRefractColor, refractionColor, 0.15);
	float planarReflectionWeight = clamp(planeReflectionStrength * 0.30, 0.0, 1.0);
	reflectionColor = mix(reflectionColor, planeReflectionColor, planarReflectionWeight);

	float NDotV = clamp(dot(N, V), 0.0, 1.0);
	float viewLift = pow(1.0 - NDotV, 1.5);
	float fresnel = pow(1.0 - NDotV, fresnelPower);
	float thinMask = smoothstep(thicknessCutoff, thicknessCutoff + 0.03, thickness);
	//vec3 refractBlue = refractionColor * mix(vec3(1.0), bodyColor, 0.45);
	vec3 refracted = planeRefractColor * transmittance * vec3(0.56, 0.92, 1.06);
	refracted = mix(refracted, planeRefractColor * bodyColor, 0.28);
	float reflectionMix = clamp(0.02 + 0.12 * fresnel, 0.0, 0.22);
	reflectionMix *= mix(0.55, 1.0, NDotV);
	reflectionMix *= thinMask;
	spec *= thinMask;

	vec3 envColor = mix(refracted, reflectionColor, reflectionMix);
	vec3 lit = envColor * (0.34 + 0.66 * wrappedNDotL) + vec3(spec);
	lit = mix(lit, lit * vec3(0.75, 0.85, 1.02), 0.20);

	float diffuse = 0.48 + 0.52 * wrappedNDotL;

	float thicknessMask = clamp(thickness * 0.12, 0.0, 1.0);
	float edgeMask = 1.0 - thicknessMask;

//	vec3 edgeColor = mix(lit, refractBlue, edgeMask * 0.06);
//	float edgeLuma = dot(edgeColor, vec3(0.2126, 0.7152, 0.0722));
//	edgeColor = mix(edgeColor, vec3(edgeLuma), edgeMask * 0.05);

	float alpha = clamp(0.62 + thickness * 0.58, 0.42, 0.96);
	vec3 finalColor = envColor * (0.62 + 0.38 * wrappedNDotL)
		+ vec3(spec)
		+ vec3(0.08, 0.14, 0.20)
		+ refracted * 0.22
		+ envColor * (0.08 * viewLift)
		+ bodyColor * 0.30;

	// fluid-only gamma lift to avoid dark-blue collapse on some GPUs without touching whole window
	//finalColor = pow(max(finalColor, vec3(0.0)), vec3(0.92));

	


	//thickness debug
//	float debugThickness = texture(fluidThicknessTexture, vUV).r;
//	float vis = debugThickness * 0.2; 
//	vis = 1.0 - exp(-debugThickness * 2.0);
//	FragColor = vec4(vec3(vis), 1.0);

	//depth debug
//	float debugDepth = texture(fluidDepthTexture, vUV).r;
//	float vis = debugDepth * 0.02;
//	FragColor = vec4(vec3(vis), 1.0);
//

   FragColor = vec4(finalColor, alpha);
}