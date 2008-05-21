#version 110

varying vec3  normal;
varying vec3  halfV;

void main()
{
	// normalize interpolated normal
    vec3 n = normalize(normal);
    // compute the dot product between normal and ldir
    float NdotL = max(dot(n,normalize(gl_LightSource[0].position.xyz)),0.0);

    if (NdotL > 0.001)
    {
        float NdotHV = max(dot(n,normalize(halfV)),0.0);
        //NdotHV = max(dot(n,normalize(gl_LightSource[0].halfVector.xyz)),0.0);

        gl_FragColor =
            ( gl_FrontLightProduct[0].diffuse * NdotL * gl_Color
              + gl_FrontLightProduct[0].specular * pow(NdotHV,gl_FrontMaterial.shininess)
            );
    }
    else
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
