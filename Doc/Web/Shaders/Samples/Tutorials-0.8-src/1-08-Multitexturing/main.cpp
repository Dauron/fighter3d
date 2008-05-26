// ************************************************
// Tutorial 8: Multi-Textures
//
// (c) 2003 by Martin Christen.
// ************************************************

#include "../cwc/aGLSL.h"
#include "../cwc/simpleGL.h"
#include <GL/glut.h>
#include <iostream>
//#include <GL/glext.h>


using namespace std;

#define FRAG_FILE "tutorial8.frag"
#define VERT_FILE "tutorial8.vert"


// -------------------------------------------------
// Multi-Texturing-Extension-specific (just 2)


#define GL_COMBINE_EXT                    0x8570
#define GL_COMBINE_RGB_EXT                0x8571
// -------------------------------------------------


// GLobals:
aShaderObject* myShader = 0;
GLuint texture1;
GLuint texture2;

//*************************************************************
//Draw one Frame (don't Swap Buffers)

void drawBox(float size)
{
 // Texture Coordinate (0,0) is top left.
 glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(0.0, 0.0); glVertex3f(0.0, size*1.0, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(size*1.0, size*1.0, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(size*1.0, 0.0, 0.0);
 glEnd();

}

void drawBoxMT(float size)
{
 // Texture Coordinate (0,0) is top left.
 glBegin(GL_QUADS);
  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 1.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 1.0);
  glVertex3f(0.0, 0.0, 0.0);

  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 0.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 0.0);
  glVertex3f(0.0, size*1.0, 0.0);

  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0, 0.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 0.0);
  glVertex3f(size*1.0, size*1.0, 0.0);

  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0, 1.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 1.0);
  glVertex3f(size*1.0, 0.0, 0.0);
 glEnd();
}


void DrawFrame(void)
{
   glTranslatef(-1.0,-1.0,0.0);

   myShader->begin();

     myShader->sendUniform1i("myTexture1", 0); // Texture Unit 0
     myShader->sendUniform1i("myTexture2", 1); // Texture Unit 1

     drawBoxMT(2.0);

   myShader->end();
}


void AppInit(void)
{
   GLExtensions Ext;

   if (!Ext.check("GL_ARB_multitexture"))
   {
        cout << "**Error: GL_ARB_multitexture isn't supported on your system!\n";
        exit(-1);
   }

   if (!init_ARB_multitexture())
   {
        cout << "**Error initializing multitexture function pointers\n";
        exit(-1);
   }


  //----------------------------------------


  myShader = DefaultShaderSetup(VERT_FILE, FRAG_FILE);


  // ---------------------------------------
  // OpenGL Settings

  //DefaultLighting();

  glEnable(GL_TEXTURE_2D);

  // LoadTexture: 24bit RGB
  char* tex = LoadTexture("../textures/texture1.raw",256,3);
  char* tex2 = LoadTexture("../textures/texture2.raw",256,3);

  // createTexture creates an OpenGL 2D Texture (defined in
  // simpleGL.cpp)

  createTexture(tex, &texture1);
  createTexture(tex2, &texture2);


  glActiveTextureARB(GL_TEXTURE0_ARB);
  glBindTexture(GL_TEXTURE_2D, texture1);
  glEnable(GL_TEXTURE_2D);
  if (!myShader->oglslEnabled()) glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
  if (!myShader->oglslEnabled())glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);

  glActiveTextureARB(GL_TEXTURE1_ARB);
  glBindTexture(GL_TEXTURE_2D, texture2);
  glEnable(GL_TEXTURE_2D);
  if (!myShader->oglslEnabled())glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
  if (!myShader->oglslEnabled())glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INCR);

}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{
  DefaultCleanup();
}

//*************************************************************
//END