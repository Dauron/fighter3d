// ************************************************
// Tutorial 3: Send uniform to Fragment Program
//
// Vertex Program: transform vertices
//
//
// Fragment Program:
//                 Color = send to Program
//
// (c) 2003-2004 by Martin Christen.
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
   myShader = shadermanager.loadfromFile("tutorial3.vert","tutorial3.frag");

   if (myShader == 0)
   {
        cout << "Can't create shader object. Exiting now.\n";
        exit(-1);
   }
}

//*************************************************************
//Draw one Frame (don't Swap Buffers)
float rr = 0.5f;
float gg = 0.3f;
float bb = 0.0f;


void DrawFrame(void)
{
   rr+=0.01; if (rr > 1.0) rr = 0.5;

   myShader->begin();

   // Draw a Teapot
   myShader->sendUniform3f("myColor", rr, gg, bb);
   glutSolidTeapot(1.0);

   // Draw a 2nd Teapot with different Uniform:
   myShader->sendUniform3f("myColor", 1.0f-rr, gg, bb);
   glTranslatef(2.0f,0.0f,0.0f);
   glutSolidTeapot(0.5);

   myShader->end();
}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{
}

//*************************************************************
//END