#version 110

uniform int lighting;
uniform int texturing;

varying vec3  normal;
varying vec3  lightDir;
varying float dist;
varying vec3  halfV;

uniform sampler2D tex;

void main()
{
	vec4 color, specular = vec4(0.0, 0.0, 0.0, 0.0);

    if (lighting >= 0)
    {
        /* The global ambient term will always be present */
	    color = gl_FrontLightModelProduct.sceneColor;

        if (lighting > 0)
        {
		    vec3 n;
            vec4 contribution;
            float NdotL, NdotHV;
            float attenuation, spotEffect;
    	    
		    // normalize interpolated normal
		    n = normalize(normal);

            if (gl_LightSource[0].position.w == 1.0)
            {
                // compute the dot product between normal and ldir
				NdotL = max(dot(n,normalize(lightDir)),0.0);
                        
                if (gl_LightSource[0].spotCosCutoff < 180.0) {
				    spotEffect = dot(normalize(gl_LightSource[0].spotDirection), normalize(-lightDir));
		            if (spotEffect > gl_LightSource[0].spotCosCutoff) {
			            spotEffect = (spotEffect - gl_LightSource[0].spotCosCutoff) / (1.0 - gl_LightSource[0].spotCosCutoff);
			            spotEffect = pow(spotEffect, gl_LightSource[0].spotExponent);
		            }
                    else
                        spotEffect = 0.0;
                }
                else
                    spotEffect = 1.0;
                        
                attenuation = spotEffect / (gl_LightSource[0].constantAttenuation +
		                                    gl_LightSource[0].linearAttenuation    * dist +
		                                    gl_LightSource[0].quadraticAttenuation * dist * dist);
            }
            else
            {
                attenuation = 1.0;
                // compute the dot product between normal and ldir
				NdotL = max(dot(n,normalize(gl_LightSource[0].position.xyz)),0.0);
            }

            if (attenuation > 0.0) {
                // ambient light
                contribution = gl_FrontLightProduct[0].ambient;
                // diffuse and specular light
                if (NdotL > 0.0) {
                    contribution += gl_FrontLightProduct[0].diffuse * NdotL;
                    NdotHV = max(dot(n,normalize(halfV)),0.0);
                    //NdotHV = max(dot(n,normalize(gl_LightSource[0].halfVector.xyz)),0.0);
                    specular += attenuation *
                    	    gl_FrontLightProduct[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
                }
                color = attenuation * contribution;
            }
            else
                color = vec4(0.0, 0.0, 0.0, 0.0);
		}
	}
	else
		color = gl_FrontMaterial.diffuse;
	
    if (texturing > 0)
        color *= texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w);
	gl_FragColor = clamp(color * gl_Color + specular, 0.0, 1.0);
}
