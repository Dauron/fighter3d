varying vec3 N;
varying vec3 L;
varying vec3 P;
varying float pattern;

uniform vec4 color0;    // "Diffuse" Color
uniform vec4 color1;    // "Ambient" Color
uniform vec4 color2;    // "Specular" Color

void main (void)
{
vec3 E = normalize(-P);
vec3 R = normalize(-reflect(L,N));

float diffuse = max(dot(N,L),0.0);
diffuse = smoothstep(diffuse, 0.0, 0.5); 

float spec = pow(max(dot(R,E),0.0), 10.0);
float spec2= pattern*spec;

gl_FragColor = color1 + color0*diffuse + color2*(spec+spec2);
}