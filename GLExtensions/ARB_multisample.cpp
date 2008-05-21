#include "ARB_multisample.h"

bool GL_init_ARB_multisample(void)
{
    return GLExtensions::Exists("GL_ARB_multisample");
}
