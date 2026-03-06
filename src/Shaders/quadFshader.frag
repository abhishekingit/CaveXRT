#version 330 core

in qVS_out {
	vec2 vUV;
	vec3 WorldPos;
	vec3 WorldNormal;
	vec3 FragPos;
	vec4 FragPosLightSpace;

} qfs_in;

out vec4 outputColor;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

uniform float ambientIntensity;
uniform float specularIntensity;
uniform float glossiness;

uniform vec3 cameraPosWorld;
uniform vec3 lightPos;
uniform bool skyboxEnabled;
uniform bool showDepthMap;
uniform bool showReflections;

uniform float width;
uniform float height;

uniform sampler2D renderTexture;
uniform samplerCube cubemaptexture;
uniform sampler2D depthMap;

float shadowMapCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
	    projCoords.y < 0.0 || projCoords.y > 1.0 ||
	    projCoords.z > 1.0) {
		return 0.0;
	}

	float closestDepth = texture(depthMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}

void main() {
	vec3 lightDir = normalize(lightPos - qfs_in.FragPosLightSpace.xyz);
	if(showDepthMap) {
		float depth = texture(depthMap, qfs_in.vUV).r;
		outputColor = vec4(vec3(depth), 1.0);
		return;
	}
	vec3 finalColor = vec3(0.0);
	if(showReflections) {
		vec2 uv = vec2(gl_FragCoord.x / width, gl_FragCoord.y / height);
		vec3 reflectionColor = texture(renderTexture, uv).rgb;
		vec3 envColor = vec3(0.0);
		if(skyboxEnabled) {
			vec3 viewDir = normalize(cameraPosWorld - qfs_in.WorldPos);
			vec3 reflectDir = reflect(-viewDir, normalize(qfs_in.WorldNormal));
			envColor = texture(cubemaptexture, reflectDir).rgb;
		}

		vec3 finalColor = mix(reflectionColor, envColor, 0.5);

	}
	else {
		vec3 norm = normalize(qfs_in.WorldNormal);
		vec3 lightDir = normalize(lightPos - qfs_in.WorldPos);
		vec3 viewDir = normalize(cameraPosWorld - qfs_in.WorldPos);
		vec3 halfVec = normalize(lightDir + viewDir);

		float cosTheta = max(dot(norm, lightDir), 0.0);
		float cosAlpha = max(dot(norm, halfVec), 0.0);

		vec3 ambientTerm = ambient * ambientIntensity;
		vec3 diffuseTerm = diffuse * cosTheta;
		vec3 specularTerm = vec3(0.0);

		if(cosTheta > 0.0) {
			specularTerm = specular * specularIntensity * pow(cosAlpha, glossiness);
		}

		float shadow = shadowMapCalculation(qfs_in.FragPosLightSpace, norm, lightDir);
		finalColor = ambient + (1.0 - shadow) * (diffuseTerm + specularTerm);

	}

	
	outputColor = vec4(finalColor, 1.0);
}