#extension GL_ARB_draw_buffers : require

//all these are in eye-space
varying vec3 position;

varying vec3 normal;
varying vec3 tangentS;
varying vec3 tangentT;

varying vec2 texCoord;

uniform sampler2D TEX_color;
uniform sampler2D TEX_bump;

void main()
{
    vec4 color = texture2D( TEX_color, texCoord);
    vec3 bump  = texture2D( TEX_bump,  texCoord).xyz * 2.0 - 1.0;

    bump = bump.x * tangentS + bump.y * tangentT + bump.z * normal;
    bump = faceforward( bump, normalize(position), bump);

    gl_FragData[0] = vec4( position, 0.0);
    gl_FragData[1] = vec4( bump * 0.5 + 0.5, gl_FrontMaterial.shininess);
    //gl_FragData[2] = color;

    if (color.a < 1.0)
        gl_FragData[2] = vec4(gl_Color.rgb * color.rgb, 1.0);
    else
        gl_FragData[2] = color;
}
