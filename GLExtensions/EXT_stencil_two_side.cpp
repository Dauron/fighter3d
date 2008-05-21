#include "EXT_stencil_two_side.h"

PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT;

bool GL_init_EXT_stencil_two_side(void)
{
    int error = aLoadExtension(PFNGLACTIVESTENCILFACEEXTPROC,glActiveStencilFaceEXT);
    return error == 0 && GL_ExtensionExists("GL_EXT_stencil_two_side");
}
