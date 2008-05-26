#ifndef SIMPLE_GL_H
#define SIMPLE_GL_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "aGLSL.h"

using namespace std;

void begin2d(float x, float y);
void end2d(void);
void mcDrawString(float x, float y, stringstream& stream);
void createTexture(char* data, GLuint* num);
char* LoadTexture(char* filename, int width=256, int nbytes=4);

void DefaultLighting(void);
aShaderObject* DefaultShaderSetup(char* vertex, char* fragment);
void DefaultCleanup(void);


//define a custom keyboard function, receiving ASCII keys:
//#define CUSTOM_KEYBOARD_FUNCTION(key) AppKeyboard(key)

//define a custom mouse event function, receiving x and y coords and button state
//x,y are integer coordinates and button is a bool, true if a button is pressed)
//#define CUSTOM_MOUSE_FUNCTION(x,y,button) AppMouse(x,y,button)

#endif

