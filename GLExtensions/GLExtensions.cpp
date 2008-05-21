#include "GLExtensions.h"
#include "ARB_fragment_shader.h"
#include "ARB_shader_objects.h"
#include "ARB_vertex_shader.h"
#include "ARB_multitexture.h"
#include "ARB_vertex_buffer_object.h"
#include "EXT_texture3D.h"
#include "EXT_stencil_wrap.h"
#include "EXT_stencil_two_side.h"

#include <cstring>

bool GLExtensions::Exists_ARB_ShaderObjects      = false;
bool GLExtensions::Exists_ARB_VertexShader       = false;
bool GLExtensions::Exists_ARB_FragmentShader     = false;
bool GLExtensions::Exists_ARB_VertexBufferObject = false;
bool GLExtensions::Exists_EXT_StencilWrap        = false;
bool GLExtensions::Exists_EXT_StencilTwoSide     = false;

bool GL_Init_Extensions(void)
{
    GLExtensions::Exists_ARB_ShaderObjects  = GL_init_ARB_shader_objects();
    GLExtensions::Exists_ARB_VertexShader   = GL_init_ARB_vertex_shader();
    GLExtensions::Exists_ARB_FragmentShader = GL_init_ARB_fragment_shader();

    GLExtensions::Exists_ARB_VertexBufferObject = GL_init_ARB_vertex_buffer_object();

    GLExtensions::Exists_EXT_StencilWrap    = GL_init_EXT_stencil_wrap();
    GLExtensions::Exists_EXT_StencilTwoSide = GL_init_EXT_stencil_two_side();

    GL_init_ARB_multitexture();
    GL_init_EXT_texture3D();

    return true;
}

char *extensions = NULL;

bool GL_ExtensionExists(const char *extensionName)
{
    if (!extensions)
    {
        extensions = strdup((char *) glGetString(GL_EXTENSIONS)); // Fetch Extension String
        int len=strlen(extensions);
        for (int i=0; i<len; i++)              // Separate It By Newline Instead Of Blank
            if (extensions[i]==' ') extensions[i]='\n';
    }

    int len = strlen(extensionName);
    char *found = strstr(extensions, extensionName);

    return found && (found[len] == 0 || found[len] == '\n');
}
