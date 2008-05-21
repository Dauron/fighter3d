#version 110

const int numLights = 3;

uniform int lighting;
uniform int texturing;

varying vec3  normal;
varying vec3  lightDir[numLights];
varying float dist    [numLights];
varying vec3  halfV   [numLights];

uniform sampler2D tex;

void main()
{
	vec4 color, specular = vec4(0.0, 0.0, 0.0, 0.0);
	
	if (lighting > 0)
	{
		vec3 n;
        vec4 contribution;
        float NdotL, NdotHV;
        float attenuation, spotEffect;
	    
		/* The global ambient term will always be present */
		color = gl_FrontLightModelProduct.sceneColor;
		/* normalize interpolated normal */
		n = normalize(normal);
	    
		for (int i=0; i < numLights; ++i)
			if (i < lighting)
			{
                if (gl_LightSource[i].position.w == 1.0)
                {
                    /* compute the dot product between normal and ldir */
				    NdotL = max(dot(n,normalize(lightDir[i])),0.0);
                    if (gl_LightSource[i].spotCosCutoff < 180.0) {
				        spotEffect = dot(normalize(gl_LightSource[i].spotDirection), normalize(-lightDir[i]));
				        if (spotEffect > gl_LightSource[i].spotCosCutoff) {
					        spotEffect = (spotEffect - gl_LightSource[i].spotCosCutoff) / (1.0 - gl_LightSource[i].spotCosCutoff);
					        spotEffect = pow(spotEffect, gl_LightSource[i].spotExponent);
				        }
                        else
                            continue; //spotEffect = 0.0;
                    }
                    else
                        spotEffect = 1.0;

                    attenuation = spotEffect / (gl_LightSource[i].constantAttenuation +
				                                gl_LightSource[i].linearAttenuation    * dist[i] +
				                                gl_LightSource[i].quadraticAttenuation * dist[i] * dist[i]);
                }
                else
                {
                    attenuation = 1.0;
                    /* compute the dot product between normal and ldir */
				    NdotL = max(dot(n,normalize(gl_LightSource[i].position.xyz)),0.0);
                }

                /* ambient light */
                contribution = gl_FrontLightProduct[i].ambient;
                /* diffuse and specular light */
                if (NdotL > 0.0) {
                    NdotHV = max(dot(n,normalize(halfV[i])),0.0);
                    //NdotHV = max(dot(n,normalize(gl_LightSource[i].halfVector.xyz)),0.0);
                    contribution += gl_FrontLightProduct[i].diffuse * NdotL;
                    specular += attenuation *
                    	gl_FrontLightProduct[i].specular * pow(NdotHV,gl_FrontMaterial.shininess);
                }
                color += attenuation * contribution;
			}
            else break;
	}
	else
		color = gl_FrontMaterial.diffuse;
	
	if (texturing > 0)
		color *= texture2D(tex,gl_TexCoord[0].st);
	gl_FragColor = clamp(color * gl_Color + specular, 0.0, 1.0);
}
