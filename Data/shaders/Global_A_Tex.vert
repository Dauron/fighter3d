#version 110

void main()
{
	gl_BackColor   = gl_FrontColor  = gl_Color;
	gl_Position    = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}
