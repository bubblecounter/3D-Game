
#version 330 core
out vec4 FragColor;

in vec2 texCord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform sampler2D shadowMap;

uniform vec3 objectColor;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightPos;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide ( make value betwen -1 and 1)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = 0.0;
	float bias = 0.00004;
	if((currentDepth-bias) > closestDepth){
		shadow = 1.0f;
	}
    return shadow;
}


void main()
{
	//ambient ligthing
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;
	//diffuse lighting
	vec3 lightDir = normalize(lightPos - FragPos); // find the direction vector
	vec3 norm = normalize(Normal); //make sure normal vector is in unit form
	float diff = max(dot(norm, lightDir), 0.0); // make sure diffuse parameter never becomes negative
	vec3 diffuse = diff * lightColor;
	//specular lighting
	float specularStrength = 0.5;
	vec3 reflectDir = reflect(-lightDir, norm);  //reflected light vector ; reflect function expects first vector from lightsource to fragment
	vec3 viewDir = normalize(viewPos - FragPos); //view direction vector

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);//make sure it never getz negative; 32 is the shininess of highlight. exponential
	vec3 specular = specularStrength * spec * lightColor;//higher the shininess higher the reflection quality
	
	// calculate shadow
   float shadow = ShadowCalculation(FragPosLightSpace); 

	//Phong Lighting
	vec3 result = (ambient + ((1.0 -shadow) *( diffuse + specular)))  * texture(texture2,texCord);

	FragColor = vec4(result, 1.0);
    //FragColor = 
}