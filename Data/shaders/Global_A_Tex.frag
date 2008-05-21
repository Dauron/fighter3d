#version 110

uniform sampler2D tex;

void main()
{
	gl_FragColor = gl_FrontLightModelProduct.sceneColor * gl_Color
        * texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w);
}
