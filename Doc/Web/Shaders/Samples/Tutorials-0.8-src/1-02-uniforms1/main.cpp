// ************************************************
// Tutorial 2: Send uniform to Vertex Program
//
// Vertex Program:
//                 (X,Y,Z) -> (0.5*X, 0.5*Y, u*Z)
//                 factor u: send to Program
//                           name "Scale"
//
// Fragment Program:
//                 Color = green
//
// (c) 2003 by Martin Christen.
// ************************************************

#include "../cwc/aGLSL.h"
#include <GL/glut.h>
#include <iostream>

using namespace std;

//Shader and Program Objects:
aShaderManager  shadermanager;
aShaderObject* myShader = 0;


void AppInit(void)
{
   myShader = shadermanager.loadfromFile("tutorial2.vert","tutorial2.frag");

   if (myShader == 0)
   {
        cout << "Can't create shader object. Exiting now.\n";
        exit(-1);
   }
}

//*************************************************************
//Draw one Frame (don't Swap Buffers)
float myVar = 0.5;

void DrawFrame(void)
{
   myVar+=0.001; if (myVar > 1.0) myVar = 0.5;

   myShader->begin();

   // Draw a Teapot
   myShader->sendUniform1f("Scale", myVar);
   glutSolidTeapot(1.0);

   // Draw a 2nd Teapot with different Uniform:
   myShader->sendUniform1f("Scale", 1-myVar);
   glTranslatef(2.0,0.0,0.0);
   glutSolidTeapot(0.5);

   myShader->end();
}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{
}

//*************************************************************