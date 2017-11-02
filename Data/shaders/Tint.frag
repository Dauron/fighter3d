varying vec2 texCoord;

uniform sampler2D TEX_image;

void main()
{
    vec3 color = texture2D(TEX_image, texCoord).rgb;
    if (color.r > gl_FrontMaterial.shininess ||
        color.g > gl_FrontMaterial.shininess ||
        color.b > gl_FrontMaterial.shininess)
        color = color * gl_FrontMaterial.ambient.rgb;

    gl_FragColor = vec4(color, 1.0);
}

