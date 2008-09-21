
attribute vec4 boneIdxWghts;
uniform   vec4 quats[32];
uniform   vec3 roots[32];
uniform   vec3 trans[32];

vec3 QuaternionRotate(vec4 q, vec3 p)
{
    vec4 a = vec4(
                  q.w * p + cross ( q.xyz , p ),
                  - dot( q.xyz , p )
                 );
    //vec4 c = vec4(
    //              q.w * a.xyz - a.w * q.xyz - cross ( a.xyz , q.xyz ),
    //              dot( q , a )
    //             );
    return q.w * a.xyz - a.w * q.xyz - cross ( a.xyz , q.xyz );
}

void getPositionAndNormal(int i, inout vec4 position, inout vec3 normal)
{
    float wght = fract(boneIdxWghts[i]);
    if (wght <= 0.001) return;
    wght *= 10.0;
    
    int idx = int(floor(boneIdxWghts[i]));
    if (idx == 0)
    {
        position.xyz += wght * (gl_Vertex.xyz + quats[0].xyz);
        position.w    = gl_Vertex.w;
        normal       += wght * gl_Normal;
        return;
    }

    if (gl_Vertex.w != 0.0)
        position.xyz += wght * (QuaternionRotate(quats[idx], gl_Vertex.xyz - roots[idx]) + trans[idx]);
    else
        position.xyz += wght * QuaternionRotate(quats[idx], gl_Vertex.xyz);
    position.w        = gl_Vertex.w;
    normal           += wght * QuaternionRotate(quats[idx], gl_Normal);
}
