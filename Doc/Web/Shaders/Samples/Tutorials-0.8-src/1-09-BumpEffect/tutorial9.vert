varying vec3 N;
varying vec3 L;
varying vec3 P;
varying float pattern;

const vec3 lightPos = vec3(0.0,5.0,5.0);  // make this uniform

void main(void)
{
  P = (gl_ModelViewMatrix * gl_Vertex).xyz;
  L = normalize(lightPos - P);
  N = normalize(gl_NormalMatrix * gl_Normal);

  pattern=fract(4.0*(gl_Vertex.y+gl_Vertex.x+gl_Vertex.z));

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  //Option: Transform Vertices too:
  //vec4 v=gl_Vertex+0.05*pattern*vec4(gl_Normal,0.0);
  //gl_Position = gl_ModelViewProjectionMatrix * v; 
 
}
