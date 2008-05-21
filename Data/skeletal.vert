#version 110

const int numLights = 3;

uniform int texturing;
uniform int lighting;

varying vec3 normal;
varying vec3 lightDir[numLights];
varying float dist[numLights];
varying vec3 halfV[numLights];

attribute vec4 boneIdxWghts;
uniform   vec4 bones[80]; // up to 40 bones

mat4 MatrixFromQuaternion(const vec4 q)
{
    mat4 res;
    res[0][0] = 1.0 - 2.0 * (q.y*q.y + q.z*q.z);
    res[0][1] = 2.0 * (q.z*q.y - q.w*q.z);
    res[0][2] = 2.0 * (q.w*q.y + q.x*q.z);
    res[0][3] = 0.0;
    res[1][0] = 2.0 * (q.x*q.y + q.w*q.z);
    res[1][1] = 1.0 - 2.0 * (q.x*q.x + q.z*q.z);
    res[1][2] = 2.0 * (q.y*q.z - q.w*q.x);
    res[1][3] = 0.0;
    res[2][0] = 2.0 * (q.x*q.z - q.w*q.y);
    res[2][1] = 2.0 * (q.y*q.z + q.w*q.x);
    res[2][2] = 1.0 - 2.0 * (q.x*q.x + q.y*q.y);
    res[2][3] = 0.0;
    res[3][0] = 0.0;
    res[3][1] = 0.0;
    res[3][2] = 0.0;
    res[3][3] = 1.0;
    return res;
}

void main()
{
	//// Skeletal animation
    mat4 transform  = mat4(0.0);
    mat4 transformN = mat4(0.0);
	vec4 position;
	int i;
	for (i = 0; i < 4; ++i) {
		float wght = 10.0*fract(boneIdxWghts[i]);
		if (wght > 0.01) {
            int idx = int(floor(boneIdxWghts[i]));
            mat4 trans  = mat4(1.0);
            mat4 transN = mat4(1.0);
            do {
                vec4 q = bones[idx*2];
                vec4 t = bones[idx*2+1];
                if (q.w < 1.0)
                {
                    mat4 m1  = mat4(1.0); m1[0][3] = -t[0]; m1[1][3] = -t[1]; m1[2][3] = -t[2];
                    mat4 m2  = mat4(1.0); m2[0][3] = +t[0]; m2[1][3] = +t[1]; m2[2][3] = +t[2];
                    mat4 mq  = MatrixFromQuaternion(q);
                    trans  *= m1 * mq * m2;
                    transN *= mq;
                }
                else if (t.w == -1.0)
                {
                    mat4 m2  = mat4(1.0); m2[0][3] = +t[0]; m2[1][3] = +t[1]; m2[2][3] = +t[2];
                    trans *= m2;
                }
                idx = int(t.w);
            } while (idx >= 0);
            transform  += wght * trans;
            transformN += wght * transN;
        }
	}
    position = gl_Vertex * transform;
    normal   = vec3(vec4(gl_Normal, 0.0) * transformN);
	
	//// Lighting
	vec3 vertex;
	vec3 aux;
	
	/* first transform the normal into eye space and normalize the result */
	normal = normalize(gl_NormalMatrix * normal);
	vertex = vec3(gl_ModelViewMatrix * position);

	for (int i=0; i < numLights; ++i) {
		if (i < lighting) {
			/* compute the light's direction */
			aux = gl_LightSource[i].position.xyz-vertex;
			lightDir[i] = normalize(aux);
			dist[i] = length(aux);
            halfV[i] = normalize(gl_LightSource[i].halfVector.xyz);
		}
		else {
			lightDir[i] = vec3(1.0,0.0,0.0);
			dist[i] = 0.0;
		}
	}
	
	//// Leave texture without changes
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
   	gl_BackColor = gl_FrontColor = gl_Color;
   	gl_Position = gl_ModelViewProjectionMatrix * position;
}
