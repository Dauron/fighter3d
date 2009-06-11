#version 110

varying vec3  normal;
varying vec3  halfV;

uniform sampler2D tex;

void main()
{
  	// normalize interpolated normal
	float N = dot(normal, normal); // vec3  n = normalize(normal);
	float L = dot(gl_LightSource[0].position.xyz, gl_LightSource[0].position.xyz);
	// compute the dot product between normal and ldir
	float NdotL = dot(normal, gl_LightSource[0].position.xyz) * inversesqrt(N * L); // = max(dot(n,normalize(lightDir)),0.0);
	if (NdotL < 0.001)
	{
		gl_FragColor = gl_FrontLightProduct[0].ambient
              * texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w);
		return;
	}
		
	vec4 color = ( gl_FrontLightProduct[0].ambient
		         + gl_FrontLightProduct[0].diffuse * NdotL
		         ) * texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w);
	
	float NdotHV = dot(normal, halfV) * inversesqrt(N * dot(halfV, halfV)); // = max(dot(n,normalize(halfV)),0.0);
	if (NdotHV > 0.0)
		color = color + 
			clamp(gl_FrontLightProduct[0].specular * pow(NdotHV,gl_FrontMaterial.shininess), 0.0, 1.0);
	
	gl_FragColor = color;
}
