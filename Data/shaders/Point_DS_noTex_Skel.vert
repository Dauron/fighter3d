#version 110

varying vec3  normal;
varying vec3  lightDir;
varying float dist;
varying vec3  halfV;

#include "skeletal.vert"

void main()
{
	//// Skeletal animation
    vec4 position = vec4(0.0,0.0,0.0,0.0);
    normal = vec3(0.0,0.0,0.0);
	getPositionAndNormal(0, position, normal);
	getPositionAndNormal(1, position, normal);
	getPositionAndNormal(2, position, normal);
	getPositionAndNormal(3, position, normal);
	
	//// Lighting
	normal = normalize(gl_NormalMatrix * normal);
	vec3 vertex = vec3(gl_ModelViewMatrix * position);
    vec3 aux = gl_LightSource[0].position.xyz-vertex;
    lightDir = normalize(aux);
    dist = length(aux);
    halfV = normalize(gl_LightSource[0].halfVector.xyz);
	
   	gl_BackColor = gl_FrontColor = gl_Color;
   	gl_Position = gl_ModelViewProjectionMatrix * position;
}
