bool init_ARB_vertex_shader(void);

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

#endif /* GL_ARB_vertex_shader */


extern PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB;
extern PFNGLGETACTIVEATTRIBARBPROC          glGetActiveAttribARB;
extern PFNGLGETATTRIBLOCATIONARBPROC        glGetAttribLocationARB;


