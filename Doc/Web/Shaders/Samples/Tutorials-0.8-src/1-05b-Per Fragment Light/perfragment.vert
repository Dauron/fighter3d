varying vec3 L;
varying vec3 N;
varying vec3 P;

void main(void)
{
  P = vec3(gl_ModelViewMatrix * gl_Vertex);
  L = normalize(gl_LightSource[0].position.xyz-P);
  N = normalize(gl_NormalMatrix * gl_Normal);

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
