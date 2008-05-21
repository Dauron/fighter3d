#include "EXT_stencil_two_side.h"

PFNGLActiveStencilFaceEXT glActiveStencilFaceEXT;

bool GL_init_EXT_stencil_two_side(void)
{
    int error = aLoadExtension(PFNGLActiveStencilFaceEXT,glActiveStencilFaceEXT);

    return (bool)error | GL_ExtensionExists("GL_EXT_stencil_two_side");
}
