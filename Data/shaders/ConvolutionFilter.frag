varying vec2 texCoord;

uniform sampler2D TEX_image;

uniform vec2 TexOffset;
uniform mat3 Mask;

void main()
{
    int radius = 1;

    vec3 accum = vec3(0.0);
    for (int x = -radius; x <= radius; ++x)
        for (int y = -radius; y <= radius; ++y)
            accum += Mask[radius+x][radius+y] * texture2D(TEX_image, texCoord + TexOffset * vec2(x,y)).xyz;

    gl_FragColor = vec4(accum, 1.0);
}

