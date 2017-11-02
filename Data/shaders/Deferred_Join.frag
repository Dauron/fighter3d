//#extension GL_ARB_draw_buffers : require

varying vec2 texCoord;

uniform sampler2D TEX_diffuse;
uniform sampler2D TEX_specular;

void main()
{
    vec4 diffuse  = texture2D( TEX_diffuse,  texCoord);
    vec4 specular = texture2D( TEX_specular, texCoord);

    gl_FragColor  = diffuse + specular;
}