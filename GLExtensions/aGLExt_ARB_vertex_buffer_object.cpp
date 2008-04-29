GLboolean use_ARB_vertex_buffer_object = GL_FALSE;

// Functions pointers for ARB_vertex_buffer_object Extension:
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;
PFNGLISBUFFERARBPROC glIsBufferARB = NULL;
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = NULL;
PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARB = NULL;
PFNGLMAPBUFFERARBPROC glMapBufferARB = NULL;
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = NULL;
PFNGLGETBUFFERPOINTERVARBPROC glGetBufferPointervARB = NULL;

bool init_ARB_vertex_buffer_object(void)
{
  int error = 0;

  error |= aLoadExtension(PFNGLBINDBUFFERARBPROC, glBindBufferARB);
  error |= aLoadExtension(PFNGLDELETEBUFFERSARBPROC, glDeleteBuffersARB);
  error |= aLoadExtension(PFNGLGENBUFFERSARBPROC, glGenBuffersARB);
  error |= aLoadExtension(PFNGLISBUFFERARBPROC, glIsBufferARB);
  error |= aLoadExtension(PFNGLBUFFERDATAARBPROC, glBufferDataARB);
  error |= aLoadExtension(PFNGLBUFFERSUBDATAARBPROC, glBufferSubDataARB);
  error |= aLoadExtension(PFNGLGETBUFFERSUBDATAARBPROC, glGetBufferSubDataARB);
  error |= aLoadExtension(PFNGLMAPBUFFERARBPROC, glMapBufferARB);
  error |= aLoadExtension(PFNGLUNMAPBUFFERARBPROC, glUnmapBufferARB);
  error |= aLoadExtension(PFNGLGETBUFFERPARAMETERIVARBPROC, glGetBufferParameterivARB);
  error |= aLoadExtension(PFNGLGETBUFFERPOINTERVARBPROC, glGetBufferPointervARB);

  return error == 0;
}
