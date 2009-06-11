#version 110

varying vec3  normal;
varying vec3  lightDir;
varying float dist;
varying vec3  halfV;

uniform sampler2D tex;

void main()
{
    float L = dot(lightDir, lightDir);
	float spotEffect = dot(gl_LightSource[0].spotDirection, -lightDir) * // (N dot L) * rsq( (N dot N) * (L dot L) ) = normalize(N) dot normalize(L)
		inversesqrt ( dot(gl_LightSource[0].spotDirection,gl_LightSource[0].spotDirection) * L);
	//float spotEffect = dot(normalize(gl_LightSource[0].spotDirection), normalize(-lightDir));
    if (spotEffect < gl_LightSource[0].spotCosCutoff) discard;
		
	spotEffect = (spotEffect - gl_LightSource[0].spotCosCutoff) / (1.0 - gl_LightSource[0].spotCosCutoff);
	spotEffect = pow(spotEffect, gl_LightSource[0].spotExponent);

	float attenuation = spotEffect / (gl_LightSource[0].constantAttenuation +
						 gl_LightSource[0].linearAttenuation    * dist +
						 gl_LightSource[0].quadraticAttenuation * dist * dist);    
	if (attenuation < 0.001) discard;

	// normalize interpolated normal
	float N = dot(normal, normal); // vec3  n = normalize(normal);
	// compute the dot product between normal and ldir
	float NdotL = dot(normal, lightDir) * inversesqrt(N * L); // = max(dot(n,normalize(lightDir)),0.0);
	if (NdotL < 0.001) discard;
		
	vec4 color = gl_FrontLightProduct[0].diffuse * NdotL
			   * texture2D(tex,gl_TexCoord[0].st/gl_TexCoord[0].w);
	
	float NdotHV = dot(normal, halfV) * inversesqrt(N * dot(halfV, halfV)); // = max(dot(n,normalize(halfV)),0.0);
	if (NdotHV > 0.0)
		color = color + 
			clamp(gl_FrontLightProduct[0].specular * pow(NdotHV,gl_FrontMaterial.shininess), 0.0, 1.0);
	
	gl_FragColor = attenuation * color;
}
