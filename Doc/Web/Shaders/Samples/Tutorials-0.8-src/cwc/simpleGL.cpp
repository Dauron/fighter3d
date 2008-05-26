/**********************************************************
This source is used to make *SIMPLE* OGLSL Demos.

Basically it is a minimal GLUT setup code.
The user/student just has to add basic OpenGL code
and GLSL variable management to test GLSL programs.

There are also some functions for

   * Loading and creating textures (24 and 32 bit RAW)
   * Drawing in 2D (for image processing OGLSL programs)
   * Simple Text Output


Last update: January 27, 2004
Version:     0.0.5
author:      Martin Christen, christen@clockworkcoders.com
**********************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "simpleGL.h"
#include <vector>

using namespace std;


// Define this in main.cpp
extern void DrawFrame(void);
extern void AppExit(void);
extern void AppInit(void);

// simple Camera:
float camera[3];
float lookat[3];

float cam[3];

// Mouse rotation (currently not trackball)
static int mousepos_x = 0, mousepos_y = 0, tmpX, tmpY;
bool rotate=false;

// other globals:
bool _use_shader = true;     // false: user turned off shader functionality.

// Default Shader
aShaderObject* _DefaultShader = 0;
aVertexShader* _DefaultVertexShader = 0;
aFragmentShader* _DefaultFragmentShader = 0;


// ---------------------------------------------------------------------------------------
// switch to 2D-Coordinate System:
// x,y: Screen coordinates to set origin in screen Space
// note: (0,0) is TOP LEFT!!

void begin2d(float x, float y)
{
    int win_width = glutGet(GLUT_WINDOW_WIDTH);
    int win_height = glutGet(GLUT_WINDOW_HEIGHT);

    glPushAttrib(GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Koordinaten 2D:
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, win_width, 0, -win_height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef((float)x, (float)y-win_height, 0.0);
    glRasterPos2f(0, 0);
}


// ---------------------------------------------------------------------------------------
// Stop drawing in Screen Coordinates and use old GL Settings

void end2d(void)
{
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

// ---------------------------------------------------------------------------------------
// mcDrawString: Draw a string in screen coordinates
// (0,0) is top left.

static GLvoid *font_style = GLUT_BITMAP_HELVETICA_18;
// GLUT_BITMAP_HELVETICA_10, GLUT_BITMAP_HELVETICA_12, GLUT_BITMAP_HELVETICA_18
// GLUT_BITMAP_TIMES_ROMAN_10, GLUT_BITMAP_TIMES_ROMAN_24
// GLUT_BITMAP_8_BY_13, GLUT_BITMAP_9_BY_15

void mcDrawString(float x, float y, stringstream& stream)
{

    begin2d(x,y);

    for (unsigned int i=0; i<(strlen(stream.str().c_str())); i++)
        glutBitmapCharacter(font_style, stream.str().c_str()[i]);

    stream.str("");

    end2d();
}

// ---------------------------------------------------------------------------------------
//Load RAW quadratic texture (24bit or 32bit)

// store all textures in a master list:
vector<char*> Texture_MasterList;

char* LoadTexture(char* filename, int width, int nbytes) // load RAW N x N Texture RGBA (4byte) or RGB (3byte)
{
    if ((nbytes !=3) && (nbytes !=4))
    {
        cout << "error: supported Textures: 24bit (3 bytes) or 32bit (4 bytes)";
        return 0;
     }

    char* optr;
    ifstream file;
    file.open(filename, ios::in | ios::binary);
    if(!file)
    {
        cout << "** FATAL ERROR: Can't open texture\n";
        exit(-1);
    }

    // get file length:
    file.seekg(0,ios::end);
    unsigned long L = file.tellg();
    file.seekg(ios::beg);

    if (L/nbytes/width!=width)
    {
        cout << file << " is not a " << width << "x" << width << " Texture with " << nbytes << " bytes per pixel\n";
        exit(-1);
    }
    optr =  new char[L+1];
    optr[0] = (char) nbytes;   // first byte stores number of bytes per pixel, 3 or 4

    file.read(optr+1, L);

    Texture_MasterList.push_back(optr);

    return optr;
}

// ---------------------------------------------------------------------------------------

// create Texture from data
void createTexture(char* data, GLuint* num)
{
    glGenTextures(1, num);
    glBindTexture(GL_TEXTURE_2D, *num);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if (data[0] == 4)  // RGBA Texture: 4 Bytes per pixel
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA ,GL_UNSIGNED_BYTE, data+1);

    else if (data[0] == 3) // RGB Texture: 3 Bytes per pixel
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB ,GL_UNSIGNED_BYTE, data+1);


}

// ----------------------------------------------------------

void init(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glShadeModel(GL_FLAT);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
    //glEnable(GL_CULL_FACE);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    AppInit();
}

// ---------------------------------------------------------------------------------------

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Modelview Transformation
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camera[0], camera[1], camera[2], lookat[0], lookat[1], lookat[2], 0, 1, 0);

    //glTranslatef(0.0,0.0,-5.0);

    glTranslatef(cam[0],cam[1],cam[2]);

    // Simple mouse rotation:
    glRotatef((GLfloat) mousepos_x, -1.0, 0.0, 0.0);
    glRotatef((GLfloat) mousepos_y, 0.0, -1.0, 0.0);



    // Call user draw function
    DrawFrame();


    glutSwapBuffers();

}

// ---------------------------------------------------------------------------------------

void reshape(int w, int h)
{
float aspect = (float) w/((h)?h:1);

    glViewport(0,0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, aspect, 1.0f, 100.0f);
    glViewport(0,0,w,h);

    glMatrixMode(GL_MODELVIEW);
}

// ---------------------------------------------------------------------------------------

void animate(void)
{
    glutPostRedisplay();
}

// ---------------------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:    // escape: exit
            AppExit();
            exit(0);
            break;

        case 'w':
            cam[1] += 0.1;
            break;

        case 's':
            cam[1] -= 0.1;
            break;

        case 'a':
            cam[0] += 0.1;
            break;

        case 'd':
            cam[0] -= 0.1;
            break;

       case 'r':
            cam[2] += 0.1;
            break;

        case 'f':
            cam[2] -= 0.1;
            break;


        case 'p':   // turn Shader on/off
            _use_shader = !_use_shader;
            aShaderObject::useShader(_use_shader);
            break;

    }

#ifdef CUSTOM_KEYBOARD_FUNCTION
  CUSTOM_KEYBOARD_FUNCTION(key);
#endif

}

// ---------------------------------------------------------------------------------------

void mouse(int button, int state, int x, int z)
{
 switch(button){
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN)
    {
      tmpX = x;
      tmpY = z;
      rotate = true;
    }
    else
    {
      rotate = false;
    }
    break;
  }

#ifdef CUSTOM_MOUSE_FUNCTION
  CUSTOM_MOUSE_FUNCTION(x,y,(state==GLUT_DOWN));
#endif

}

// ---------------------------------------------------------------------------------------

void motion(int x, int z)
{
  if(rotate)   //Rotiere, wenn linke maustaste gedrückt
  {
    mousepos_y = (mousepos_y + (x - tmpX))%360;
    mousepos_x = (mousepos_x + (z - tmpY))%360;
    tmpX = x;
    tmpY = z;
    glutPostRedisplay();
  }

}

// ---------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{

 atexit(DefaultCleanup);

 camera[0] = 0.0f;
 camera[1] = 0.0f;
 camera[2] = 5.0f;

 cam[0] = cam[1] = cam[2] = 0.0f;

 lookat[0] = 0.0f-camera[0];
 lookat[1] = 0.0f-camera[1];
 lookat[2] = 5.0f-camera[2];


 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
 glutInitWindowSize(640,480);
 glutInitWindowPosition(0,0);
 glutCreateWindow(argv[0]);
 init();
 glutDisplayFunc(display);
 glutReshapeFunc(reshape);
 glutKeyboardFunc(keyboard);
 //glutSpecialFunc(specialkeys);
 glutMouseFunc(mouse);
 glutMotionFunc(motion);
 glutIdleFunc(animate);
 glutMainLoop();

 AppExit();

 return 0;
}


// ---------------------------------------------------------------------------------------
// Setup a 'nice' default lighting
// you can call this function in main.cpp
void DefaultLighting(void)
{
  // ---------------------------------------
  // OpenGL Settings
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE);



  // Light model parameters:
  // -------------------------------------------

  GLfloat lmKa[] = {0.0, 0.0, 0.0, 0.0 };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmKa);

  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0);

  // -------------------------------------------
  // Spotlight Attenuation

  GLfloat spot_direction[] = {1.0, -1.0, -1.0 };
  GLint spot_exponent = 30;
  GLint spot_cutoff = 180;

  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
  glLighti(GL_LIGHT0, GL_SPOT_EXPONENT, spot_exponent);
  glLighti(GL_LIGHT0, GL_SPOT_CUTOFF, spot_cutoff);

  GLfloat Kc = 1.0;
  GLfloat Kl = 0.0;
  GLfloat Kq = 0.0;

  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,Kc);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, Kl);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, Kq);



  // -------------------------------------------
  // Lighting parameters:

  GLfloat light_pos[] = { 0.0, 5.0, 5.0, 1.0};
  GLfloat light_Ka[] = {1.0, 0.5, 0.5, 1.0};
  GLfloat light_Kd[] = {0.1, 0.1, 0.1, 1.0};
  GLfloat light_Ks[] = {1.0, 1.0, 1.0, 1.0};

  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_Ka);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_Kd);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_Ks);

  // -------------------------------------------
  // Material parameters:

  GLfloat material_Ka[] = {0.5, 0.0, 0.0, 1.0};
  GLfloat material_Kd[] = {0.4, 0.4, 0.5, 1.0};
  GLfloat material_Ks[] = {0.8, 0.8, 0.0, 1.0};
  GLfloat material_Ke[] = {0.1, 0.0, 0.0, 0.0};
  GLfloat material_Se = 20.0;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);

}


// ---------------------------------------------------------------------------------------
// ShaderSetup loads a vertex and a fragment shader
// and returns the Object. (You can set Fragment or Vertex
// Filename to 0 and default shader is used.)
// I don't want load / compile / link source code in every
// tutorial main file when I don't need OGLSL libraries.

aShaderObject* DefaultShaderSetup(char* vertex, char* fragment)
{
  _DefaultShader = new aShaderObject;
  if (vertex!=0) _DefaultVertexShader = new aVertexShader;
  if (fragment!=0) _DefaultFragmentShader = new aFragmentShader;


  // load vertex program
  if (vertex!=0)
  if (_DefaultVertexShader->load(vertex) != 0)
  {
    cout << "can't load vertex shader!\n";
    exit(-1);  // if file is missing: major error, better exit
  }

  // Load fragment program
  if (fragment!=0)
  if (_DefaultFragmentShader->load(fragment) != 0)
  {
    cout << "can't load fragment shader!\n";
    exit(-1); // if file is missing: major error, better exit
  }

  // Compile vertex program
  if (vertex!=0)
  if (!_DefaultVertexShader->compile())
  {
      cout << "***COMPILER ERROR (Vertex Shader):\n";
      cout << _DefaultVertexShader->getCompilerLog() << endl;
      exit(-1);
  }

  // Compile fragment program
  if (fragment!=0)
  if (!_DefaultFragmentShader->compile())
  {
     cout << "***COMPILER ERROR (Fragment Shader):\n";
     cout << _DefaultFragmentShader->getCompilerLog() << endl;
     exit(-1);
  }

  // Add to object
  if (vertex!=0) _DefaultShader->addShader(_DefaultVertexShader);
  if (fragment!=0) _DefaultShader->addShader(_DefaultFragmentShader);

  // link
  if (!_DefaultShader->link())
  {
     cout << "**LINKER ERROR\n";
     cout << _DefaultShader->getLinkerLog() << endl;
     exit(-1);
  }

  return _DefaultShader;
}



// ---------------------------------------------------------------------------------------
// exit handler calls DefaultCleanup
void DefaultCleanup(void)
{
  // free default Shaders if defined
  if (_DefaultShader!=0) delete _DefaultShader; _DefaultShader=0;
  if (_DefaultVertexShader!=0) delete _DefaultVertexShader; _DefaultVertexShader=0;
  if (_DefaultFragmentShader!=0) delete _DefaultFragmentShader; _DefaultFragmentShader=0;

  // free textures
  vector<char*>::iterator i = Texture_MasterList.begin();
  while (i!=Texture_MasterList.end())
  {
     free(*i);
     i = Texture_MasterList.erase(i);
  }
}

