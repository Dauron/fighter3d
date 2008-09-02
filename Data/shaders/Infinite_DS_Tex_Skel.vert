#version 110

varying vec3  normal;
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
    halfV  = normalize(gl_LightSource[0].halfVector.xyz);
	
	//// Leave texture without changes
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
   	gl_BackColor = gl_FrontColor = gl_Color;
   	gl_Position = gl_ModelViewProjectionMatrix * position;
}
