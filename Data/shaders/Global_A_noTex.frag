#version 110

void main()
{
	gl_FragColor = gl_FrontLightModelProduct.sceneColor * gl_Color;
}
