// ************************************************
// Light.cpp
// C++ Program: Basic Fragment Lighting
//
// (c) 2003-2004 by Martin Christen
//
// ************************************************

#include "../cwc/aGLSL.h"
#include "../cwc/simpleGL.h"
#include <GL/glut.h>
#include <iostream>
#include <math.h>

using namespace std;

#define FRAG_FILE "perfragment.frag"
#define VERT_FILE "perfragment.vert"

void sendProgramColor(int preset);

// GLobals:
aShaderObject* myShader = 0;


//*************************************************************
// Draw a primitive (Torus) at x,y,z
int deg =0; // little animation
void primitive(float x, float y, float z)
{
   glPushMatrix();
   glTranslatef(x,y,z);
   glRotatef(deg,1.0,x,y);

   //glutSolidTorus(0.2,0.5,48,48);
   glColor4f(0.5, 0.0, 0.0, 1.0);
   //glutSolidSphere(0.6,32,32);
   glutSolidTeapot(1.0);
   //glutSolidCube(0.5);
   glPopMatrix();
}




//*************************************************************
//Draw one Frame (don't Swap Buffers)
float f=0.0;
float u=0.0;

void DrawFrame(void)
{
   float xx=0.0,yy=0.0,zz=0.0;
   xx = 5+cos(u - 0.785);
   yy = 5+cos(2.0*u - 0.785); u=u+0.1;
   zz = 5.0;

   GLfloat light_pos[] = { xx, yy, zz, 1.0};
   glPushMatrix();
   glLoadIdentity();
   glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
   glPopMatrix();

   myShader->begin();
   //myShader->sendUniform1f("f", f);

   primitive(2.0,  0.0, 0.0);
   primitive(0.0,  2.0, 0.0);
   primitive(-2.0,  0.0, 0.0);
   primitive(0.0,  -2.0, 0.0);

   glRotatef(-90,1.0,1.0,0.0);
   glutSolidSphere(0.7,64,64);
   //glutSolidCube(1.2);

   myShader->end();
}


void AppInit(void)
{
  glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

  // Load Shaders
  myShader = DefaultShaderSetup(VERT_FILE, FRAG_FILE);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_NORMALIZE);

  // OpenGL Settings

  DefaultLighting();
}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{
  DefaultCleanup();
}


//END