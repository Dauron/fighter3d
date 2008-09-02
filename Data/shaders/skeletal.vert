
attribute vec4 boneIdxWghts;
uniform   vec4 quats[32];
uniform   vec3 roots[32];
uniform   vec3 trans[32];

vec4 QuaternionProduct(vec4 a, vec4 b) 
{
  vec4 ret;
  ret.x = a.y*b.z - a.z*b.y + a.w*b.x + a.x*b.w;
  ret.y = a.z*b.x - a.x*b.z + a.w*b.y + a.y*b.w;
  ret.z = a.x*b.y - a.y*b.x + a.w*b.z + a.z*b.w;
  ret.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  return ret;
}

vec3 QuaternionRotate(vec4 q, vec3 p)
{
    vec4 ret;
    ret.x =   q.y*p.z - q.z*p.y + q.w*p.x;
    ret.y =   q.z*p.x - q.x*p.z + q.w*p.y;
    ret.z =   q.x*p.y - q.y*p.x + q.w*p.z;
    ret.w = - q.x*p.x - q.y*p.y - q.z*p.z;
    return QuaternionProduct(ret, vec4(-q.x,-q.y,-q.z,q.w)).xyz;
}

void getPositionAndNormal(int i, inout vec4 position, inout vec3 normal)
{
    position.w = gl_Vertex.w;
    
    float wght = 10.0*fract(boneIdxWghts[i]);
	if (wght > 0.01) {
		int idx = int(floor(boneIdxWghts[i]));

		if (idx == 0)
        {
			position.xyz += wght * (gl_Vertex.xyz + quats[0].xyz);
            normal       += wght * gl_Normal;
        }
		else
		{
            if (gl_Vertex.w != 0.0)
                position.xyz += wght * (QuaternionRotate(quats[idx], gl_Vertex.xyz - roots[idx]) + trans[idx]);
            else
                position.xyz += wght * QuaternionRotate(quats[idx], gl_Vertex.xyz);
			normal           += wght * QuaternionRotate(quats[idx], gl_Normal).xyz;
		}
    }
}

