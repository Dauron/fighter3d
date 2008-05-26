// ************************************************
// Tutorial 4: varying
//
//
// (c) 2003-2004 by Martin Christen.
// ************************************************

#include "../cwc/aGLSL.h"
#include <GL/glut.h>
#include <iostream>

using namespace std;

//Shader and Program Objects:
aShaderManager shadermanager;
aShaderObject* myShader = 0;



//*************************************************************
//Draw one Frame
void DrawFrame(void)
{
   myShader->begin();
   glutSolidCube(2.0);
   myShader->end();
}

//*************************************************************

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
// use App Exit to clean up
void AppExit(void)
{

}

//*************************************************************
