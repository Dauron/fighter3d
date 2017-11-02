
//all these are in eye-space
varying vec3 position;

varying vec3 normal;
varying vec3 tangentS;
varying vec3 tangentT;

varying vec2 texCoord;

void main()
{
    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;

    position = (gl_ModelViewMatrix * gl_Vertex).xyz;

    //transform tangents and normal
    normal   = gl_NormalMatrix * gl_Normal;
    tangentS = gl_NormalMatrix * gl_MultiTexCoord1.xyz;
    tangentT = gl_NormalMatrix * gl_MultiTexCoord2.xyz;

    texCoord = gl_MultiTexCoord0.st;
}