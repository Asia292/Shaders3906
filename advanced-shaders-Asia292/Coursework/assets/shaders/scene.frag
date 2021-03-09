#version 330

// Input interface block
in VS_OUT
{
	in vec3 FragPos;
	in vec2 TextCoord;
	in vec3 FragNormal;
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
}fs_in;

// Light source attribute structure
struct LightAttr
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform LightAttr light;
uniform bool normalMapping;

// Textures in the model
uniform sampler2D texture_diffuse0; 
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;

out vec4 color;

void main()
{   
	// Ambient light component
	vec3	ambient = light.ambient * vec3(texture(texture_diffuse0, fs_in.TextCoord));
	vec3    viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	// Diffuse reflected light component
	vec3    lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	vec3	normal = normalize(fs_in.FragNormal);

	if(normalMapping)
	{
		normal = texture(texture_normal0, fs_in.TextCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0);
	}

	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse0, fs_in.TextCoord));
	float	specFactor = 0.0;
	vec3	halfDir = normalize(lightDir + viewDir);
	specFactor = pow(max(dot(halfDir, normal), 0.0), 64.0);
	vec3	specular = specFactor * light.specular * vec3(texture(texture_specular0, fs_in.TextCoord));;

	vec3	result = (ambient + diffuse + specular );
	color	= vec4(result , 1.0f);
}