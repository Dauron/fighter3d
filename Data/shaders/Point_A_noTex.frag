#version 110

varying float dist;

void main()
{
    float attenuation = 1.0 / (gl_LightSource[0].constantAttenuation +
                               gl_LightSource[0].linearAttenuation    * dist +
                               gl_LightSource[0].quadraticAttenuation * dist * dist);
    if (attenuation < 0.001) discard;
	
	gl_FragColor = attenuation * gl_FrontLightProduct[0].ambient;
}
