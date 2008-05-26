
uniform sampler2D myTexture;

void main (void)
{
  gl_FragColor = texture2D(myTexture, vec2(gl_TexCoord[0]));
}