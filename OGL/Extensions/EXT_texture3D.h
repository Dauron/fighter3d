#include "GLExtensions.h"

bool GL_init_EXT_texture3D(void);

#ifndef GL_EXT_texture3D
#define GL_EXT_texture3D                 1

#define GL_TEXTURE_BINDING_3D            0x806A
#define GL_PACK_SKIP_IMAGES              0x806B
#define GL_PACK_IMAGE_HEIGHT             0x806C
#define GL_UNPACK_SKIP_IMAGES            0x806D
#define GL_UNPACK_IMAGE_HEIGHT           0x806E
#define GL_TEXTURE_3D                    0x806F
#define GL_PROXY_TEXTURE_3D              0x8070
#define GL_TEXTURE_DEPTH                 0x8071
#define GL_TEXTURE_WRAP_R                0x8072
#define GL_MAX_3D_TEXTURE_SIZE           0x8073


typedef GLvoid (APIENTRY * PFNGLTEXIMAGE3DEXTPROC)(GLenum target, GLint level, GLenum internalformat,
                                                   GLsizei width, GLsizei height, GLsizei depth,
                                                   GLint border, GLenum format, GLenum type,
                                                   const GLvoid* pixels);

typedef GLvoid (APIENTRY * PFNGLTEXSUBIMAGE3DPROC)(GLenum target, GLint lod,
                                                   GLint xoffset, GLint yoffset, GLint zoffset,
                                                  GLsizei w, GLsizei h, GLsizei d,
                                                   GLenum format, GLenum type, const GLvoid *buf);

typedef GLvoid (APIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC)(GLenum target, GLint level, GLint xoffset,
                                                       GLint yoffset, GLint zoffset, GLint x,
                                                       GLint y, GLsizei width, GLsizei height);

#endif /* GL_EXT_texture3D */

#ifndef GL_VERSION_1_2

extern PFNGLTEXIMAGE3DEXTPROC     glTexImage3D;
extern PFNGLTEXSUBIMAGE3DPROC     glTexSubImage3D;
extern PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D;

#endif
