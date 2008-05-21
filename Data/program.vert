#version 110

uniform int lighting;
uniform int texturing;

varying vec3  normal;
varying vec3  lightDir;
varying float dist;
varying vec3  halfV;

void main()
{
	//// Lighting
	vec3 vertex;
	
	/* first transform the normal into eye space and normalize the result */
	normal = normalize(gl_NormalMatrix * gl_Normal);
	vertex = vec3(gl_ModelViewMatrix * gl_Vertex);

    if (lighting > 0)
    {
        /* compute the light's direction */
		vec3 aux = gl_LightSource[0].position.xyz-vertex;
		lightDir = normalize(aux);
		dist = length(aux);
        halfV = normalize(gl_LightSource[0].halfVector.xyz);
	}
	
	//// Leave texture without changes
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
   	gl_BackColor   = gl_FrontColor  = gl_Color;
	gl_Position    = ftransform(); //gl_ModelViewProjectionMatrix * gl_Vertex;
}
