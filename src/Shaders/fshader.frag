#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
	
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

void main() {
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfVec = normalize(lightDir + viewDir);

	float cosTheta = max(dot(norm, lightDir), 0.0);
	float cosAlpha = max(dot(norm, halfVec), 0.0);

	vec3 baseColor = material.hasDiffuseMap ? texture(material.map_Kd, TexCoords).rgb : material.diffuse;

	vec3 ambient = baseColor * material.ambientIntensity;
	vec3 diffuse = baseColor * cosTheta * light.color;

	vec3 specMask = material.hasSpecularMap ? texture(material.map_Ks, TexCoords).rgb : material.specular; 
	vec3 specularColor = material.specularIntensity * specMask;
	vec3 specular = vec3(0.0);
	if(cosTheta > 0.0) {
		specular =  specularColor * pow(cosAlpha, material.glossiness) * light.color;
	}
		

	outputColor = vec4(ambient + diffuse + specular, 1.0);
//	outputColor = vec4(fract(TexCoords), 0.0, 1.0);
}