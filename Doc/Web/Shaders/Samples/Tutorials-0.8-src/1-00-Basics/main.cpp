// ****************************************************
// Simple program to test GLSL Shaders
//
// This Program shows how to load a Shader from memory
//
// Implement the functions:
//        AppInit() to initialize
//        AppExit() to clean up
// and    DrawFrame() to draw one frame
// ****************************************************

#include "../cwc/aGLSL.h"
#include <GL/glut.h>
#include <GL/gl.h>

aShaderManager  shadermanager;
aShaderObject*  shader;

static const char* myVertexShader =
"void main(void)                                     \n"
"{                                                   \n"
"    vec4 a = vec4(1.0,0.5,1.0,1.0) * gl_Vertex;      \n"
"    gl_Position = gl_ModelViewProjectionMatrix * a;  \n"
"}                                                   \n\0";

static const char* myFragmentShader =
"void main (void)                                    \n"
"{                                                   \n"
"    gl_FragColor = vec4 (0.0, 1.0, 0.0, 1.0);        \n"
"}                                                   \n\0";

//*************************************************************
// at the time of App init, glut is already initialized
// and standard OpenGL settings are done. You could test
// extensions etc now!

void AppInit(void)
{
  shader = shadermanager.loadfromMemory(myVertexShader,myFragmentShader);

  if (shader==0)
  {
      std::cout << "error: can't init shader!\n";
      exit(-1);
  }

  //shader = shadermanager.loadfromFile("test.vert","test.frag");
}

//*************************************************************
//Draw one Frame (don't Swap Buffers)
void DrawFrame(void)
{
    shader->begin();
    glutSolidTeapot(1.5);
    shader->end();
}


//*************************************************************
// use App Exit to clean up
void AppExit(void)
{

}

//*************************************************************