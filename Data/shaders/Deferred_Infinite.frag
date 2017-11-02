#extension GL_ARB_draw_buffers : require

varying vec2 texCoord;

uniform sampler2D TEX_position;
uniform sampler2D TEX_normal;
uniform sampler2D TEX_material;

void main()
{
    vec3 position = texture2D( TEX_position, texCoord).xyz;
    vec4 bump     = texture2D( TEX_normal,   texCoord);
    vec4 material = texture2D( TEX_material, texCoord);
    vec3 normal   = bump.xyz * 2.0 - 1.0;

    if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0) // sky
    {
        gl_FragData[0] = material;
        gl_FragData[1] = vec4(0.0);
        return;
    }

    // normalize interpolated normal
    vec3 L = normalize(gl_LightSource[0].position.xyz);
    float NdotL = dot(normal, L);
    if (NdotL < 0.001)
    {
        gl_FragData[0] = vec4(material.rgb * gl_LightSource[0].ambient.rgb, material.a);
        gl_FragData[1] = vec4(0.0);
        return;
    }

    vec3 specular = vec3(0.0);

    if (bump.w > 0.0)
    {
        vec3 R = reflect(normalize(position), normal);
        float RdotL = pow( max( 0.0, dot(R, L)), bump.w);
        specular = RdotL * mix( material.rgb, vec3(1.0), 0.2);
    }

    gl_FragData[0] = vec4(material.rgb *
                          ( gl_LightSource[0].ambient.rgb
                          + NdotL * gl_LightSource[0].diffuse.rgb ), material.a);
    gl_FragData[1] = vec4(specular * gl_LightSource[0].specular.rgb, 0.0);
}