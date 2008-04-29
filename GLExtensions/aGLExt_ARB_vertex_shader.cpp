

// Functions pointers for ARB_vertex_shader Extension:

PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB     = NULL;
PFNGLGETACTIVEATTRIBARBPROC          glGetActiveAttribARB        = NULL;
PFNGLGETATTRIBLOCATIONARBPROC        glGetAttribLocationARB      = NULL;

PFNGLVERTEXATTRIB4FPROC              glVertexAttrib4f               = NULL;
PFNGLVERTEXATTRIB4FVPROC             glVertexAttrib4fv              = NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC      glVertexAttribPointerARB       = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glEnableVertexAttribArrayARB   = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC  glDisableVertexAttribArrayARB  = NULL;


bool init_ARB_vertex_shader(void)
{
  int error = 0;

  error |= aLoadExtension(PFNGLBINDATTRIBLOCATIONARBPROC,glBindAttribLocationARB);
  error |= aLoadExtension(PFNGLGETACTIVEATTRIBARBPROC,glGetActiveAttribARB);
  error |= aLoadExtension(PFNGLGETATTRIBLOCATIONARBPROC,glGetAttribLocationARB);

  error |= aLoadExtension(PFNGLVERTEXATTRIB4FPROC,glVertexAttrib4f);
  error |= aLoadExtension(PFNGLVERTEXATTRIB4FVPROC,glVertexAttrib4fv);
  error |= aLoadExtension(PFNGLVERTEXATTRIBPOINTERARBPROC,glVertexAttribPointerARB);
  error |= aLoadExtension(PFNGLENABLEVERTEXATTRIBARRAYARBPROC,glEnableVertexAttribArrayARB);
  error |= aLoadExtension(PFNGLENABLEVERTEXATTRIBARRAYARBPROC,glDisableVertexAttribArrayARB);

  return error == 0;
}

