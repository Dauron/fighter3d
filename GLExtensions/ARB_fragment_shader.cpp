#include "ARB_fragment_shader.h"

bool GL_init_ARB_fragment_shader(void)
{
    return GLExtensions::Exists("GL_ARB_fragment_shader");
}
