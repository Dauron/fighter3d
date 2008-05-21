#version 110

varying float dist;

uniform sampler2D tex;

void main()
{
    float attenuation = 1.0 / (gl_LightSource[0].constantAttenuation +
                               gl_LightSource[0].linearAttenuation    * dist +
                               gl_LightSource[0].quadraticAttenuation * dist * dist);

    if (attenuation > 0.001)
        gl_FragColor = attenuation * gl_FrontLightProduct[0].ambient
            * texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w);
    else
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
