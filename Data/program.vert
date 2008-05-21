#version 110

const int numLights = 3;

uniform int lighting;
uniform int texturing;

varying vec3  normal;
varying vec3  lightDir[numLights];
varying float dist    [numLights];
varying vec3  halfV   [numLights];

void main()
{
	//// Lighting
	vec3 vertex;
	vec3 aux;
	
	/* first transform the normal into eye space and normalize the result */
	normal = normalize(gl_NormalMatrix * gl_Normal);
	vertex = vec3(gl_ModelViewMatrix * gl_Vertex);

	for (int i=0; i < numLights; ++i) {
		if (i < lighting) {
			/* compute the light's direction */
			aux = gl_LightSource[i].position.xyz-vertex;
			lightDir[i] = normalize(aux);
			dist[i] = length(aux);
            halfV[i] = normalize(gl_LightSource[i].halfVector.xyz);
		}
		else {
            break;
			//lightDir[i] = vec3(1.0,0.0,0.0);
			//dist[i] = 0.0;
		}
	}
	
	//// Leave texture without changes
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
   	gl_BackColor   = gl_FrontColor  = gl_Color;
	gl_Position    = ftransform(); //gl_ModelViewProjectionMatrix * gl_Vertex;
}
