varying vec3 vNormal;
varying vec3 vVertex;

uniform vec4 color0;    // Material Color: 0.8, 0.0, 0.0, 1.0
uniform vec4 color1;    // Silhouette Color: black
uniform vec4 color2;    // Specular Color: 0.8, 0.0, 0.0, 1.0


#define shininess 20.0



void main (void)
{

// Lighting
vec3 eyePos = vec3(0.0,0.0,5.0);
vec3 lightPos = vec3(0.0,5.0,5.0);

vec3 EyeVert = normalize(eyePos - vVertex);
vec3 LightVert = normalize(lightPos - vVertex);
vec3 EyeLight = normalize(LightVert+EyeVert);
vec3 Normal = normalize(gl_NormalMatrix * vNormal);

// Simple Silhouette
float sil = max(dot(Normal,EyeVert), 0.0);
if (sil < 0.3) gl_FragColor = color1;
else 
{
 
 gl_FragColor = color0 ; //* sil;

 // Specular part
 float spec = pow(max(dot(Normal,EyeLight),0.0), shininess);

 if (spec < 0.2) gl_FragColor *= 0.8;
 else gl_FragColor = color2; 
 
 //gl_FragColor += color2*smoothstep(0.1,0.4,spec);
 //gl_FragColor += color2*step(0.4,spec);
 
 //gl_FragColor += spec;  // advanced 3D Toon

 // Diffuse part
 float diffuse = max(dot(Normal,LightVert),0.0);
 if (diffuse < 0.5) gl_FragColor *=0.8;


}








}