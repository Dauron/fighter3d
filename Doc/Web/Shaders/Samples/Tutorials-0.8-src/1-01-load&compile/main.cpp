//*************************************************************
// Tutorial 1: Load simple Fragment/Vertex Program
//
// Vertex Program: (X,Y,Z) -> (0.5*X, 0.5*Y, Z)
// Fragment Program: Color = green
//
// (c) 2003-2004 by Martin Christen.
//*************************************************************

#include "../cwc/aGLSL.h"
#include <GL/glut.h>
#include <iostream>

using namespace std;

//Shader and Program Objects:
aShaderManager  shadermanager;
aShaderObject* myShader = 0;

//*************************************************************
void AppInit(void)
{

   myShader = shadermanager.loadfromFile("tutorial1.vert","tutorial1.frag");

   if (myShader == 0)
   {
        cout << "Can't create shader object. Exiting now.\n";
        exit(-1);
   }
}

//*************************************************************
//Draw one Frame (don't Swap Buffers)
void DrawFrame(void)
{
   myShader->begin();
   glutSolidTeapot(1.0);
   myShader->end();
}

//*************************************************************
// use App Exit to clean up
void AppExit(void)
{

}

//*************************************************************
//END