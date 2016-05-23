#version  150

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

out vec3 fN;
out vec3 fE;
out vec3 fL;
out vec2 texCoord;

uniform mat4 model_view;
uniform mat4 projection;
uniform vec4 LightPosition;


void main()
{
vec3 pos=(model_view*vPosition).xyz;

fN=(model_view*vec4(vNormal,0.0)).xyz;
fE=-pos;
fL=LightPosition.xyz;

if(LightPosition.w!=0.0){
fL=LightPosition.xyz-pos;
}

texCoord=vTexCoord;
gl_Position = projection*model_view*vPosition;
}