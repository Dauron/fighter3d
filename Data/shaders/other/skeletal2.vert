#version 110

uniform int lighting;
uniform int texturing;

varying vec3  normal;
varying vec3  lightDir;
varying float dist;
varying vec3  halfV;

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

vec4 QuaternionProduct(vec4 a, vec4 b) 
{
  vec4 ret;
  ret.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  ret.x = a.y*b.z - a.z*b.y + a.w*b.x + a.x*b.w;
  ret.y = a.z*b.x - a.x*b.z + a.w*b.y + a.y*b.w;
  ret.z = a.x*b.y - a.y*b.x + a.w*b.z + a.z*b.w;
  return ret;
}

vec4 QuaternionRotate(vec4 q, vec4 p)
{
    vec4 q_minus = vec4(-q.x,-q.y,-q.z,q.w);
    return QuaternionProduct(QuaternionProduct(q, p), q_minus);
}

void main()
{
	//// Skeletal animation
    vec4 position = vec4(0.0,0.0,0.0,0.0);
	int i;
	for (i = 0; i < 4; ++i) {
		float wght = 10.0*fract(boneIdxWghts[i]);
		if (wght > 0.01) {
		    int idx = int(floor(boneIdxWghts[i]));
            vec4 pos = gl_Vertex;
            vec4 nrm = vec4(gl_Normal, 0.0);
            do {
                vec4 q = bones[idx*2];
                vec4 t = bones[idx*2+1];
                if (q.w < 1.0)
                {
                    pos = QuaternionRotate(q, pos-t) + t;
                    nrm = QuaternionRotate(q, nrm);
                }
                else if (t.w == -1.0)
                {
                    pos += t;
                }
                idx = int(t.w);
            } while (idx >= 0);
            position += wght * pos;
            normal   += wght * vec3(nrm);
        }
	}
    normal = normalize(normal);
	
	//// Lighting
	vec3 vertex;
	
	/* first transform the normal into eye space and normalize the result */
	normal = normalize(gl_NormalMatrix * normal);
	vertex = vec3(gl_ModelViewMatrix * position);

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
   	gl_BackColor = gl_FrontColor = gl_Color;
   	gl_Position = gl_ModelViewProjectionMatrix * position;
}
