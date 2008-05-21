#version 110

varying vec3  lightDir;
varying float dist;

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
							 
	gl_FragColor = attenuation * gl_FrontLightProduct[0].ambient;
}
