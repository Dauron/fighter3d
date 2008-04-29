

// Functions pointers for ARB_shader_objects Extension:

PFNGLDELETEOBJECTARBPROC                    glDeleteObjectARB           = NULL;
PFNGLGETHANDLEARBPROC                       glGetHandleARB              = NULL;
PFNGLDETACHOBJECTARBPROC                    glDetachObjectARB           = NULL;
PFNGLCREATESHADEROBJECTARBPROC              glCreateShaderObjectARB     = NULL;
PFNGLSHADERSOURCEARBPROC                    glShaderSourceARB           = NULL;
PFNGLCOMPILESHADERARBPROC                   glCompileShaderARB          = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC             glCreateProgramObjectARB    = NULL;
PFNGLATTACHOBJECTARBPROC                    glAttachObjectARB           = NULL;
PFNGLLINKPROGRAMARBPROC                     glLinkProgramARB            = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC                glUseProgramObjectARB       = NULL;
PFNGLVALIDATEPROGRAMARBPROC                 glValidateProgramARB        = NULL;

PFNGLUNIFORM1FARBPROC                       glUniform1fARB              = NULL;
PFNGLUNIFORM2FARBPROC                       glUniform2fARB              = NULL;
PFNGLUNIFORM3FARBPROC                       glUniform3fARB              = NULL;
PFNGLUNIFORM4FARBPROC                       glUniform4fARB              = NULL;
PFNGLUNIFORM1IARBPROC                       glUniform1iARB              = NULL;
PFNGLUNIFORM2IARBPROC                       glUniform2iARB              = NULL;
PFNGLUNIFORM3IARBPROC                       glUniform3iARB              = NULL;
PFNGLUNIFORM4IARBPROC                       glUniform4iARB              = NULL;
PFNGLUNIFORM1FVARBPROC                      glUniform1fvARB             = NULL;
PFNGLUNIFORM2FVARBPROC                      glUniform2fvARB             = NULL;
PFNGLUNIFORM3FVARBPROC                      glUniform3fvARB             = NULL;
PFNGLUNIFORM4FVARBPROC                      glUniform4fvARB             = NULL;
PFNGLUNIFORM1IVARBPROC                      glUniform1ivARB             = NULL;
PFNGLUNIFORM2IVARBPROC                      glUniform2ivARB             = NULL;
PFNGLUNIFORM3IVARBPROC                      glUniform3ivARB             = NULL;
PFNGLUNIFORM4IVARBPROC                      glUniform4ivARB             = NULL;
PFNGLUNIFORMMATRIX2FVARBPROC                glUniformMatrix2fvARB       = NULL;
PFNGLUNIFORMMATRIX3FVARBPROC                glUniformMatrix3fvARB       = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC                glUniformMatrix4fvARB       = NULL;

PFNGLGETOBJECTPARAMETERFVARBPROC            glGetObjectParameterfvARB   = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC            glGetObjectParameterivARB   = NULL;
PFNGLGETINFOLOGARBPROC                      glGetInfoLogARB             = NULL;
PFNGLGETATTACHEDOBJECTSARBPROC              glGetAttachedObjectsARB     = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC              glGetUniformLocationARB     = NULL;
PFNGLGETACTIVEUNIFORMARBPROC                glGetActiveUniformARB       = NULL;
PFNGLGETUNIFORMFVARBPROC                    glGetUniformfvARB           = NULL;
PFNGLGETUNIFORMIVARBPROC                    glGetUniformivARB           = NULL;
PFNGLGETSHADERSOURCEARBPROC                 glGetShaderSourceARB        = NULL;

bool init_ARB_shader_objects(void)
{
  int error = 0;

  error |= aLoadExtension(PFNGLDELETEOBJECTARBPROC        , glDeleteObjectARB        );
  error |= aLoadExtension(PFNGLGETHANDLEARBPROC           , glGetHandleARB           );
  error |= aLoadExtension(PFNGLDETACHOBJECTARBPROC        , glDetachObjectARB        );
  error |= aLoadExtension(PFNGLCREATESHADEROBJECTARBPROC  , glCreateShaderObjectARB  );
  error |= aLoadExtension(PFNGLSHADERSOURCEARBPROC        , glShaderSourceARB        );
  error |= aLoadExtension(PFNGLCOMPILESHADERARBPROC       , glCompileShaderARB       );
  error |= aLoadExtension(PFNGLCREATEPROGRAMOBJECTARBPROC , glCreateProgramObjectARB );
  error |= aLoadExtension(PFNGLATTACHOBJECTARBPROC        , glAttachObjectARB        );
  error |= aLoadExtension(PFNGLLINKPROGRAMARBPROC         , glLinkProgramARB         );
  error |= aLoadExtension(PFNGLUSEPROGRAMOBJECTARBPROC    , glUseProgramObjectARB    );
  error |= aLoadExtension(PFNGLVALIDATEPROGRAMARBPROC     , glValidateProgramARB     );
  error |= aLoadExtension(PFNGLGETOBJECTPARAMETERFVARBPROC, glGetObjectParameterfvARB);
  error |= aLoadExtension(PFNGLGETOBJECTPARAMETERIVARBPROC, glGetObjectParameterivARB);
  error |= aLoadExtension(PFNGLGETINFOLOGARBPROC          , glGetInfoLogARB          );
  error |= aLoadExtension(PFNGLGETATTACHEDOBJECTSARBPROC  , glGetAttachedObjectsARB  );
  error |= aLoadExtension(PFNGLGETUNIFORMLOCATIONARBPROC  , glGetUniformLocationARB  );
  error |= aLoadExtension(PFNGLGETACTIVEUNIFORMARBPROC    , glGetActiveUniformARB    );
  error |= aLoadExtension(PFNGLGETUNIFORMFVARBPROC        , glGetUniformfvARB        );
  error |= aLoadExtension(PFNGLGETUNIFORMIVARBPROC        , glGetUniformivARB        );
  error |= aLoadExtension(PFNGLGETSHADERSOURCEARBPROC     , glGetShaderSourceARB     );

  error |= aLoadExtension(PFNGLUNIFORM1FARBPROC           , glUniform1fARB);
  error |= aLoadExtension(PFNGLUNIFORM2FARBPROC           , glUniform2fARB);
  error |= aLoadExtension(PFNGLUNIFORM3FARBPROC           , glUniform3fARB);
  error |= aLoadExtension(PFNGLUNIFORM4FARBPROC           , glUniform4fARB);
  error |= aLoadExtension(PFNGLUNIFORM1IARBPROC           , glUniform1iARB);
  error |= aLoadExtension(PFNGLUNIFORM2IARBPROC           , glUniform2iARB);
  error |= aLoadExtension(PFNGLUNIFORM3IARBPROC           , glUniform3iARB);
  error |= aLoadExtension(PFNGLUNIFORM4IARBPROC           , glUniform4iARB);
  error |= aLoadExtension(PFNGLUNIFORM1FVARBPROC          , glUniform1fvARB);
  error |= aLoadExtension(PFNGLUNIFORM2FVARBPROC          , glUniform2fvARB);
  error |= aLoadExtension(PFNGLUNIFORM3FVARBPROC          , glUniform3fvARB);
  error |= aLoadExtension(PFNGLUNIFORM4FVARBPROC          , glUniform4fvARB);
  error |= aLoadExtension(PFNGLUNIFORM1IVARBPROC          , glUniform1ivARB);
  error |= aLoadExtension(PFNGLUNIFORM2IVARBPROC          , glUniform2ivARB);
  error |= aLoadExtension(PFNGLUNIFORM3IVARBPROC          , glUniform3ivARB);
  error |= aLoadExtension(PFNGLUNIFORM4IVARBPROC          , glUniform4ivARB);
  error |= aLoadExtension(PFNGLUNIFORMMATRIX2FVARBPROC    , glUniformMatrix2fvARB);
  error |= aLoadExtension(PFNGLUNIFORMMATRIX3FVARBPROC    , glUniformMatrix3fvARB);
  error |= aLoadExtension(PFNGLUNIFORMMATRIX4FVARBPROC    , glUniformMatrix4fvARB);

  return error == 0;
}
