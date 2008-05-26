// ************************************************
// Tutorial 10: Toon Shading
//
// (c) 2003 by Martin Christen.
// ************************************************

#include "../cwc/aGLSL.h"
#include "../cwc/simpleGL.h"
#include <GL/glut.h>
#include <iostream>
//#include <GL/glext.h>


using namespace std;

#define FRAG_FILE "tutorial10.frag"
#define VERT_FILE "tutorial10.vert"

void ToonColor(int preset);

// GLobals:
aShaderObject* myShader = 0;


//*************************************************************
// Draw a primitive (Torus) at x,y,z
int deg =0; // little animation
void primitive(float x, float y, float z)
{
   glPushMatrix();
   glTranslatef(x,y,z);
   glRotatef(deg++,1.0,x,y); if (deg > 359) deg = 0;  // #todo time-based
   glutSolidTorus(0.2,0.5,32,32);
   glPopMatrix();
}




//*************************************************************
//Draw one Frame (don't Swap Buffers)

void DrawFrame(void)
{
   myShader->begin();
   ToonColor(0); primitive(2.0,  0.0, 0.0);
   ToonColor(1); primitive(-2.0, 0.0, 0.0);
   ToonColor(2); primitive(0.0,  2.0, 0.0);
   ToonColor(3); primitive(0.0, -2.0, 0.0);
   ToonColor(4); primitive(0.0,  0.0, 2.0);
   ToonColor(5); primitive(0.0,  0.0, -2.0);
   ToonColor(6); glutSolidTeapot(1.0);
   myShader->end();
}


void AppInit(void)
{
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // Load Shaders
  myShader = DefaultShaderSetup(VERT_FILE, FRAG_FILE);

  // OpenGL Settings

  DefaultLighting();
}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{
  DefaultCleanup();
}

//*************************************************************
// send a Color Setup to Fragment Program (0-6)
void ToonColor(int preset)
{
    switch (preset)
    {
        case 0: // red default
              myShader->sendUniform4f("color0",0.8,0.0,0.0,1.0);
              myShader->sendUniform4f("color1",0.0,0.0,0.0,1.0);
              myShader->sendUniform4f("color2",0.8,0.0,0.0,1.0);
              break;

        case 1:
              myShader->sendUniform4f("color0",0.0,0.8,0.0,1.0);
              myShader->sendUniform4f("color1",0.0,0.0,0.0,1.0);
              myShader->sendUniform4f("color2",0.0,0.8,0.0,1.0);
              break;
        case 2:
              myShader->sendUniform4f("color0",0.0,0.0,0.8,1.0);
              myShader->sendUniform4f("color1",0.0,0.0,0.0,1.0);
              myShader->sendUniform4f("color2",0.0,0.0,0.8,1.0);
              break;
        case 3:
              myShader->sendUniform4f("color0",0.8,0.0,0.8,1.0);
              myShader->sendUniform4f("color1",0.0,0.0,0.0,1.0);
              myShader->sendUniform4f("color2",0.8,0.0,0.8,1.0);
              break;
        case 4:
              myShader->sendUniform4f("color0",0.8,0.8,0.0,1.0);
              myShader->sendUniform4f("color1",0.0,0.0,0.0,1.0);
              myShader->sendUniform4f("color2",0.8,0.8,0.0,1.0);
              break;
        case 5:
              myShader->sendUniform4f("color0",0.0,0.8,0.8,1.0);
              myShader->sendUniform4f("color1",0.0,0.0,0.0,1.0);
              myShader->sendUniform4f("color2",0.0,0.8,0.8,1.0);
              break;
        case 6:
             myShader->sendUniform4f("color0",0.5,0.5,0.5,1.0);
             myShader->sendUniform4f("color1",0.3,0.3,0.3,1.0);
             myShader->sendUniform4f("color2",0.6,0.6,0.6,1.0);
             break;


    }
}
//END