#include "EXT_stencil_two_side.h"

PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT;

bool GL_init_EXT_stencil_two_side(void)
{
    int error = aLoadExtension(PFNGLACTIVESTENCILFACEEXTPROC,glActiveStencilFaceEXT);
    return error == 0 && GLExtensions::Exists("GL_EXT_stencil_two_side");
}
