#version 150

in vec3 fN;
in vec3 fE;
in vec3 fL;
in vec2 texCoord;

uniform vec4 AmbientProduct;
uniform vec4 DiffuseProduct;
uniform vec4 SpecularProduct;
uniform float Shininess;
uniform sampler2D texture;



void main()
{
vec3 N=normalize(fN);
vec3 E=normalize(fE);
vec3 L=normalize(fL);
vec3 H=normalize(L+E);


vec4 ambient=AmbientProduct;

float Kd=max(dot(L,N),0.0);
vec4 diffuse=Kd*DiffuseProduct;

float Ks=pow(max(dot(N,H),0.0),Shininess);
vec4 specular=Ks*SpecularProduct;

if(dot(L,N)<0.0){
specular=vec4(0.0,0.0,0.0,1.0);
}

gl_FragColor=(ambient+diffuse+specular)*texture2D(texture,texCoord);

}