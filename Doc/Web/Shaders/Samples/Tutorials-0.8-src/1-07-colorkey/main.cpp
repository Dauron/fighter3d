// ************************************************
// Tutorial 7: Color Key
//
// (c) 2003 by Martin Christen.
// ************************************************

#include "../cwc/aGLSL.h"
#include "../cwc/simpleGL.h"
#include <GL/glut.h>
#include <iostream>

using namespace std;

#define FRAG_FILE "tutorial7.frag"
#define VERT_FILE "tutorial7.vert"


// GLobals:
aShaderObject* myShader = 0;
GLuint TextureHandle = 0;


//*************************************************************
//Draw one Frame (don't Swap Buffers)
void DrawFrame(void)
{


   myShader->begin();
   myShader->sendUniform1i("myTexture", 0);
   glutSolidTeapot(1.0);
   myShader->end();
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


  // createTexture creates an OpenGL Texture (defined in
  // simpleGL.cpp)
  createTexture(tex, &TextureHandle);
}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{
  DefaultCleanup();
}

//*************************************************************
//END