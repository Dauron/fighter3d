#include "GLExtensions.h"
#include "ARB_shader_objects.h"

bool GL_init_ARB_vertex_shader(void);

#ifndef GL_ARB_vertex_shader
#define GL_ARB_vertex_shader                        1

#define GL_VERTEX_SHADER_ARB                        0x8B31

#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB        0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB                   0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB       0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB     0x8B4D

#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB             0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB   0x8B8A


typedef GLvoid (APIENTRY *PFNGLBINDATTRIBLOCATIONARBPROC)(GLhandleARB programObj,
                                                          GLuint index,
                                                          const GLcharARB *name);
typedef GLvoid (APIENTRY *PFNGLGETACTIVEATTRIBARBPROC)(GLhandleARB programObj,
                                                       GLuint index,
                                                       GLsizei maxLength,
                                                       GLsizei *length,
                                                       GLint *size,
                                                       GLenum *type,
                                                       GLcharARB *name);
typedef GLint (APIENTRY *PFNGLGETATTRIBLOCATIONARBPROC)(GLhandleARB programObj,
                                                       const GLcharARB *name);

typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4FPROC) (GLuint index,
                                                      GLfloat v0,
                                                      GLfloat v1,
                                                      GLfloat v2,
                                                      GLfloat v3);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4FVPROC) (GLuint index,
                                                      const GLfloat *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIBPOINTERARBPROC)(GLuint index,
                                                      GLint size,
                                                      GLenum type,
                                                      GLboolean normalized,
                                                      GLsizei stride,
                                                      const GLvoid *pointer);
typedef GLvoid (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYARBPROC)(GLuint index);

#endif /* GL_ARB_vertex_shader */

extern PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB;
extern PFNGLGETACTIVEATTRIBARBPROC          glGetActiveAttribARB;
extern PFNGLGETATTRIBLOCATIONARBPROC        glGetAttribLocationARB;

extern PFNGLVERTEXATTRIB4FPROC              glVertexAttrib4f;
extern PFNGLVERTEXATTRIB4FVPROC             glVertexAttrib4fv;
extern PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glDisableVertexAttribArrayARB;
