uniform float Scale;
uniform float TEST;

void main(void)
{
	vec4 a = gl_Vertex;
	a.x = a.x * 0.5;
	a.y = a.y * 0.5;
	a.z = a.z * Scale;

	gl_Position = gl_ModelViewProjectionMatrix * a;
}
