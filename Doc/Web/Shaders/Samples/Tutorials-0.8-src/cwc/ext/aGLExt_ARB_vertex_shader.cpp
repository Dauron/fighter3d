

// Functions pointers for ARB_vertex_shader Extension:

PFNGLBINDATTRIBLOCATIONARBPROC       glBindAttribLocationARB     = NULL;
PFNGLGETACTIVEATTRIBARBPROC          glGetActiveAttribARB        = NULL;
PFNGLGETATTRIBLOCATIONARBPROC        glGetAttribLocationARB      = NULL;



bool init_ARB_vertex_shader(void)
{
  int error = 0;

  error |= aLoadExtension(PFNGLBINDATTRIBLOCATIONARBPROC,glBindAttribLocationARB);
  error |= aLoadExtension(PFNGLGETACTIVEATTRIBARBPROC,glGetActiveAttribARB);
  error |= aLoadExtension(PFNGLGETATTRIBLOCATIONARBPROC,glGetAttribLocationARB);

  if (error) return false;

  return true;
}

