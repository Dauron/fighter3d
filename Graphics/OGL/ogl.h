#ifndef __incl_OGL_ogl_h
#define __incl_OGL_ogl_h

#ifdef WIN32
#   include "glew/include/GL/glew.h"
#   include "glew/include/GL/wglew.h"
#
#   pragma comment(lib, "opengl32.lib")
#   pragma comment(lib, "glu32.lib")
#   pragma comment(lib, "glew32.lib")
#
#else
#
#   include <GL/glew.h>
#
#endif

#endif

