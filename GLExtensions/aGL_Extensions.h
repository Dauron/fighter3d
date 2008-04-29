
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#include <GL/gl.h>
#define aLoadExtension(functype,funcname) ((funcname = (functype) wglGetProcAddress( #funcname )) == NULL)
#else
#include <GL/gl.h>
#include <GL/glx.h>
#define aLoadExtension(functype,funcname) ((funcname = (functype) glXGetProcAddressARB( (const GLubyte*) #funcname )) == NULL)
#endif

#include "aGLExt_ARB_fragment_shader.h"
#include "aGLExt_ARB_shader_objects.h"
#include "aGLExt_ARB_vertex_shader.h"
#include "aGLExt_ARB_multitexture.h"
#include "aGLExt_ARB_vertex_buffer_object.h"
#include "aGLExt_EXT_texture3D.h"

extern bool agl_ShaderObjectsLoaded;
extern bool agl_VertexShaderLoaded;
extern bool agl_FragmentShaderLoaded;
extern bool agl_VBOLoaded;

bool init_ARB_extensions(void);
