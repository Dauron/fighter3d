#version 110

varying vec3  normal;
varying vec3  halfV;

void main()
{
   	gl_BackColor = gl_FrontColor  = gl_Color;
	gl_Position  = ftransform();

	normal = normalize(gl_NormalMatrix * gl_Normal);
    halfV  = normalize(gl_LightSource[0].halfVector.xyz);
}
