// ************************************************
// Tutorial 6: Textures
//
// (c) 2003 by Martin Christen.
// ************************************************

#include "../cwc/aGLSL.h"
#include "../cwc/simpleGL.h"
#include <GL/glut.h>
#include <iostream>

using namespace std;

#define FRAG_FILE "tutorial6.frag"
#define VERT_FILE "tutorial6.vert"


// GLobals:
aShaderObject* myShader = 0;
GLuint texture1;
GLuint texture2;

//*************************************************************
//Draw one Frame (don't Swap Buffers)
void DrawFrame(void)
{
   //myShader->begin();
   //myShader->sendUniform1i("myTexture", 0); // Texture Unit 0

   glBindTexture(GL_TEXTURE_2D, texture1);
   glutSolidTeapot(1.0);


   glTranslatef(2.0,0.0,0.0);
   glBindTexture(GL_TEXTURE_2D, texture2);
   glutSolidTeapot(0.5);
   //myShader->end();

}





void AppInit(void)
{
  myShader = DefaultShaderSetup(VERT_FILE, FRAG_FILE);


  // ---------------------------------------
  // OpenGL Settings

  //DefaultLighting();

  glEnable(GL_TEXTURE_2D);

  // LoadTexture is definied in simpleGL.cpp, it
  // loads a 256x256 RAW RGBA (32bit) texture.
  char* tex = LoadTexture("../textures/texture256x256RGBA.raw");
  char* tex2 = LoadTexture("../textures/texture2-256x256RGBA.raw");

  // createTexture creates an OpenGL 2D Texture (defined in
  // simpleGL.cpp)

  createTexture(tex, &texture1);
  createTexture(tex2, &texture2);
}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{
  DefaultCleanup();
}

//*************************************************************
//END