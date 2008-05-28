#include "GLExtensions.h"

bool GL_init_EXT_stencil_two_side(void);

#ifndef GL_EXT_stencil_two_side
#define GL_EXT_stencil_two_side                 1

typedef GLvoid (APIENTRY * PFNGLACTIVESTENCILFACEEXTPROC) (GLenum face);

#define  GL_STENCIL_TEST_TWO_SIDE_EXT          0x8910
#define  GL_ACTIVE_STENCIL_FACE_EXT            0x8911

#endif /* GL_EXT_stencil_two_side */

extern PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT;
