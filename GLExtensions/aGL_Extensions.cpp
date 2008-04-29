#include "aGL_Extensions.h"

bool agl_ShaderObjectsLoaded  = false;
bool agl_VertexShaderLoaded   = false;
bool agl_FragmentShaderLoaded = false;
bool agl_VBOLoaded            = false;

#ifdef WIN32
#include "aGLExt_ARB_multitexture.cpp"
#include "aGLExt_EXT_texture3D.cpp"
#endif

#include "aGLExt_ARB_fragment_shader.cpp"
#include "aGLExt_ARB_shader_objects.cpp"
#include "aGLExt_ARB_vertex_shader.cpp"
#include "aGLExt_ARB_vertex_buffer_object.cpp"

bool init_ARB_extensions(void)
{
    agl_ShaderObjectsLoaded  = init_ARB_shader_objects();
    agl_VertexShaderLoaded   = init_ARB_vertex_shader();
    agl_FragmentShaderLoaded = init_ARB_fragment_shader();

    agl_VBOLoaded            = init_ARB_vertex_buffer_object();

#ifdef WIN32
    init_ARB_multitexture();
    init_EXT_texture3D();
#endif

    return true;
}
