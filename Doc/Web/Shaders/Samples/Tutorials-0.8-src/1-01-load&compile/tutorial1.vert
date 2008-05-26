

void main(void)
{
	vec4 a = gl_Vertex;
	a.x = a.x * 0.5;
	a.y = a.y * 0.5;	// a *= vec4(0.5,0.5,1.0,1.0);

	gl_Position = gl_ModelViewProjectionMatrix * a;
}
