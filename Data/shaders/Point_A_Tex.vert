#version 110

varying float dist;

void main()
{
   	gl_BackColor = gl_FrontColor  = gl_Color;
	gl_Position  = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	vec3 vertex = vec3(gl_ModelViewMatrix * gl_Vertex);
    dist = length(gl_LightSource[0].position.xyz-vertex);
}
