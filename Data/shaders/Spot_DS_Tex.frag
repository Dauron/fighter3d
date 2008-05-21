#version 110

varying vec3  normal;
varying vec3  lightDir;
varying float dist;
varying vec3  halfV;

uniform sampler2D tex;

void main()
{
    float spotEffect = dot(normalize(gl_LightSource[0].spotDirection), normalize(-lightDir));
    if (spotEffect > gl_LightSource[0].spotCosCutoff) {
        spotEffect = (spotEffect - gl_LightSource[0].spotCosCutoff) / (1.0 - gl_LightSource[0].spotCosCutoff);
        spotEffect = pow(spotEffect, gl_LightSource[0].spotExponent);

        float attenuation = spotEffect / (gl_LightSource[0].constantAttenuation +
                             gl_LightSource[0].linearAttenuation    * dist +
                             gl_LightSource[0].quadraticAttenuation * dist * dist);    

         // normalize interpolated normal
        vec3  n = normalize(normal);
        // compute the dot product between normal and ldir
        float NdotL = max(dot(n,normalize(lightDir)),0.0);

        if (attenuation > 0.001 && NdotL > 0.001)
        {
            float NdotHV = max(dot(n,normalize(halfV)),0.0);
            //NdotHV = max(dot(n,normalize(gl_LightSource[0].halfVector.xyz)),0.0);

            gl_FragColor = attenuation * 
                ( gl_FrontLightProduct[0].diffuse * NdotL * gl_Color
                    * texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w)
                  + gl_FrontLightProduct[0].specular * pow(NdotHV,gl_FrontMaterial.shininess)
                );
        }
        else
            gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
    else
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
