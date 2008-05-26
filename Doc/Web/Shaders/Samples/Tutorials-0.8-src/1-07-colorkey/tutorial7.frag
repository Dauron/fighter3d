
uniform sampler2D myTexture;

void main (void)
{

    vec4 value = texture2D(myTexture, vec2(gl_TexCoord[0]));
	
    if (value[3] != 0.0)
           discard;
    
	gl_FragColor = value;
}