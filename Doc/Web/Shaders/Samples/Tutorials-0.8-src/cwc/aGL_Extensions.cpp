#include "aGL_Extensions.h"




#include "ext/aGLExt_ARB_fragment_shader.cpp"

#include "ext/aGLExt_ARB_shader_objects.cpp"

#include "ext/aGLExt_ARB_vertex_shader.cpp"

#include "ext/aGLExt_ARB_multitexture.cpp"

#include "ext/aGLExt_ARB_vertex_buffer_object.cpp"

#include "ext/aGLExt_EXT_texture3D.cpp"



bool init_ARB_extensions(void)
{
    init_ARB_multitexture();
    init_ARB_vertex_shader();
    init_ARB_shader_objects();
    init_ARB_fragment_shader();
    init_ARB_vertex_buffer_object();
    init_EXT_texture3D();

    return true;
}
