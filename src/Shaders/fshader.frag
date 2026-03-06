#version 330 core

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
	vec3 WorldPos;
	vec3 WorldNormal;
} fs_in;
	
out vec4 outputColor;


struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float ambientIntensity;
	float specularIntensity;
	float glossiness;

	bool hasDiffuseMap;
	bool hasSpecularMap;
	bool hasBumpMap;

	sampler2D map_Ka;
	sampler2D map_Kd;
	sampler2D map_Ks;
};

struct Light {
	vec3 position;
	vec3 color;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform vec3 cameraPosWorld;

uniform bool isReflectionPass;
uniform mat4 reflectionMatrix;

uniform bool skyboxEnabled;
uniform samplerCube skybox;

uniform sampler2DShadow shadowMap;

float shadowMapCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
//	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//	projCoords = projCoords * 0.5 + 0.5;
//
//	if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
//	    projCoords.y < 0.0 || projCoords.y > 1.0 ||
//	    projCoords.z > 1.0) {
//		return 0.0;
//	}
	
	float bias = max(0.005 * (1.0 - dot(normalize(normal), lightDir)), 0.0005);
	vec4 proj = fragPosLightSpace;
	proj.xyz = proj.xyz * 0.5 + proj.w * 0.5;

	proj.z -= bias * proj.w;


	float shadow = textureProj(shadowMap, proj);
	//float closestDepth = texture(shadowMap, projCoords.xy).r;
	//float currentDepth = projCoords.z;
	//float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.01);
	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}

void main() {
	vec3 norm = normalize(fs_in.Normal);
	vec3 lightDir = normalize(light.position - fs_in.FragPos);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfVec = normalize(lightDir + viewDir);

	float cosTheta = max(dot(norm, lightDir), 0.0);
	float cosAlpha = max(dot(norm, halfVec), 0.0);

	//vec3 baseColor = material.hasDiffuseMap ? texture(material.map_Kd, fs_in.TexCoords).rgb * material.diffuse : material.diffuse;
	vec3 baseColor = material.diffuse;

	vec3 ambient = baseColor * material.ambientIntensity;
	vec3 diffuse = baseColor * cosTheta * light.color;

	//vec3 specMask = material.hasSpecularMap ? texture(material.map_Ks, fs_in.TexCoords).rgb : material.specular; 
	vec3 specMask = material.specular;
	vec3 specularColor = material.specularIntensity * specMask;
	vec3 specular = vec3(0.0);
	if(cosTheta > 0.0) {
		specular =  specularColor * pow(cosAlpha, material.glossiness) * light.color;
	}

	float shadow = shadowMapCalculation(fs_in.FragPosLightSpace, norm, lightDir);

	vec3 blinnShading = ambient + (shadow) * (diffuse + specular);

	vec3 reflectedColor = vec3(0.0);
	if(skyboxEnabled) {
		vec3 worldViewDir = normalize(cameraPosWorld - fs_in.WorldPos);
		vec3 worldNormal = normalize(fs_in.WorldNormal);
		vec3 reflectDir = reflect(-normalize(worldViewDir), worldNormal);
		if(isReflectionPass) {
			reflectDir.y = -reflectDir.y; // Invert Y for reflection pass
		}

		reflectedColor = texture(skybox, reflectDir).rgb;
	}
		
	vec3 finalColor = skyboxEnabled ? mix(blinnShading, reflectedColor, 0.5) : blinnShading;
	outputColor = vec4(finalColor, 1.0);
//	outputColor = vec4(fract(TexCoords), 0.0, 1.0);
}