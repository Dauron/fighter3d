#version 110

varying float dist;

#include "skeletal.vert"

void main()
{
	//// Skeletal animation
    vec4 position = vec4(0.0,0.0,0.0,0.0);
    vec3 normal = vec3(0.0,0.0,0.0);
	getPositionAndNormal(0, position, normal);
	getPositionAndNormal(1, position, normal);
	getPositionAndNormal(2, position, normal);
	getPositionAndNormal(3, position, normal);
	
	//// Lighting
	vec3 vertex = vec3(gl_ModelViewMatrix * position);
    dist = length(gl_LightSource[0].position.xyz-vertex);
	
	//// Leave texture without changes
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
   	gl_BackColor = gl_FrontColor = gl_Color;
   	gl_Position = gl_ModelViewProjectionMatrix * position;
}
