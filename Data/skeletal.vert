#version 110

uniform int lighting;
uniform int texturing;

varying vec3  normal;
varying vec3  lightDir;
varying float dist;
varying vec3  halfV;

attribute vec4 boneIdxWghts;
uniform   vec4 bones[80]; // up to 40 bones

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

void getPositionAndNormal(int i, inout vec4 position, inout vec3 normal)
{
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
                pos.xyz += t.xyz;
            }
            idx = int(t.w);
        } while (idx >= 0);
        position += wght * pos;
        normal   += wght * vec3(nrm);
    }
}

void main()
{
	//// Skeletal animation
    vec4 position = vec4(0.0,0.0,0.0,0.0);
    normal = vec3(0.0,0.0,0.0);
	getPositionAndNormal(0, position, normal);
	getPositionAndNormal(1, position, normal);
	getPositionAndNormal(2, position, normal);
	getPositionAndNormal(3, position, normal);
	
	//// Lighting
	vec3 vertex;
	
	/* first transform the normal into eye space and normalize the result */
	vertex = vec3(gl_ModelViewMatrix * position);
	normal = normalize(gl_NormalMatrix * normal);

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
