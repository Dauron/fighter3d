
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#endif


#include <GL/gl.h>

#include "ext/aGLExt_ARB_fragment_shader.h"
#include "ext/aGLExt_ARB_shader_objects.h"
#include "ext/aGLExt_ARB_vertex_shader.h"
#include "ext/aGLExt_ARB_multitexture.h"
#include "ext/aGLExt_ARB_vertex_buffer_object.h"
#include "ext/aGLExt_EXT_texture3D.h"


bool init_ARB_extensions(void);
