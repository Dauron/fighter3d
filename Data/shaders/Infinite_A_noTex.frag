#version 110

void main()
{
	gl_FragColor = gl_FrontLightProduct[0].ambient * gl_Color;
}
