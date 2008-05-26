/*
    OGLS Implementation: Per Vertex Lighting
 
    (c) 2004 by Martin Christen. All Rights Reserved.
*/


varying vec4 color;

void main(void)
{
  vec3 P = vec3(gl_ModelViewMatrix * gl_Vertex);
  vec3 L = normalize((gl_LightSource[0].position.xyz)-P);

  vec3 N = normalize(gl_NormalMatrix * gl_Normal);
  vec3 E = normalize(-P);
  vec3 H = normalize(E+L);

  vec4 Iamb = gl_FrontLightProduct[0].ambient; 
  vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N,L), 0.0);
  vec4 Ispec = gl_FrontLightProduct[0].specular * pow(max(dot(N,H),0.0),gl_FrontMaterial.shininess);

  color =  gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
