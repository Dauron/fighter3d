#ifndef GLExtensions_GLExtensions_h
#define GLExtensions_GLExtensions_h

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#include <GL/gl.h>
#define aLoadExtension(functype,funcname) ((funcname = (functype) wglGetProcAddress( #funcname )) == NULL)
#else
#include <GL/gl.h>
#include <GL/glx.h>
#define aLoadExtension(functype,funcname) ((funcname = (functype) glXGetProcAddressARB( (const GLubyte*) #funcname )) == NULL)
#endif

struct GLExtensions
{
    static bool Exists_ARB_ShaderObjects;
    static bool Exists_ARB_VertexShader;
    static bool Exists_ARB_FragmentShader;
    static bool Exists_ARB_VertexBufferObject;
    static bool Exists_EXT_StencilWrap;
    static bool Exists_EXT_StencilTwoSide;
};

bool GL_ExtensionExists(const char *extensionName);
bool GL_Init_Extensions(void);

#endif
