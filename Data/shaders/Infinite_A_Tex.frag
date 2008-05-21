#version 110

uniform sampler2D tex;

void main()
{
	gl_FragColor = gl_FrontLightProduct[0].ambient * gl_Color
        * texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w);
}
