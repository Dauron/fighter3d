#version 110

void main()
{
	gl_BackColor = gl_FrontColor  = gl_Color;
	gl_Position  = ftransform();
}
