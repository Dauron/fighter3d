#extension GL_ARB_draw_buffers : require

//all these are in eye-space
varying vec3 position;
varying vec3 normal;

void main()
{
    gl_FragData[0] = vec4( position, 0.0);
    gl_FragData[1] = vec4( normalize(normal) * 0.5 + 0.5, gl_FrontMaterial.shininess);
    gl_FragData[2] = gl_Color;
}
