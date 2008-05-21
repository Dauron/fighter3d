#include "EXT_texture3D.h"

#ifdef WIN32
PFNGLTEXIMAGE3DEXTPROC glTexImage3D = NULL;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = NULL;
PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D = NULL;
#endif

bool GL_init_EXT_texture3D(void)
{
  int error = 0;

  error |= aLoadExtension(PFNGLTEXIMAGE3DEXTPROC, glTexImage3D);
  error |= aLoadExtension(PFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);
  error |= aLoadExtension(PFNGLCOPYTEXSUBIMAGE3DPROC, glCopyTexSubImage3D);

  return error == 0;
}

