/********************************************************************
aGLSL.cpp

Version: 0.7.0 beta


(c) 2003-2004 by Martin Christen. All Rights reserved.
    christen@clockworkcoders.com
*********************************************************************/

#include "aGLSL.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <GL/glu.h>
#include <math.h>

#include "aGL_Extensions.cpp"

using namespace std;

bool useGLSL = false;


//-----------------------------------------------------------------------------
// Error Strings
char* aGLSLErrorString[] = {
        "(e0000) GLSL not enabled",
        "(e0001) not a valid program object",
        "(e0002) not a valid object",
        "(e0003) out of memory",
        "(e0004) unknown compiler error"};
//-----------------------------------------------------------------------------

// GL ERROR CHECK
int CheckGLError(char *file, int line)
{
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        cout << "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line << endl;
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}
#define CHECK_GL_ERROR() CheckGLError(__FILE__, __LINE__)


//-----------------------------------------------------------------------------

bool initGLSL(void)
{
 int error = 0;

  if (useGLSL) return true;  // already initialized

   if (!init_ARB_fragment_shader()) error = -1;
   if (!init_ARB_vertex_shader()) error = -1;;
   if (!init_ARB_shader_objects()) error = -1;;


  if (error)
  {
    useGLSL = false;
  }
  else
  {
    useGLSL = true;
  }

return useGLSL;
}

//-----------------------------------------------------------------------------

// Test if a certain Extension exists, returns true if it exists
// example: mcTextExtension("ARB_vertex_shader");
bool mcTestExtension(const char* extension_name)
{
    const GLubyte *str;
    str = glGetString(GL_EXTENSIONS);

    if (strstr((const char *)str, extension_name) != 0)  return true;

#ifdef GLSL_WINDOWS
#ifdef USE_WGLEXT
    PFNWGLGETEXTENSIONSSTRINGARBPROC   wglGetExtensionsStringARB;
    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");

    if(wglGetExtensionsStringARB == NULL)
    {
       return false;
    }

    str = (const GLubyte*) wglGetExtensionsStringARB(wglGetCurrentDC());
    if (strstr((const char *)str, extension_name) != 0) return true;
#endif
#else
// ignore additional platform extensions
#endif

    return false;
}

// ************************************************************************
// GLExtensions : Create a STL Vector with all available GLExtensions names
// ************************************************************************

GLExtensions::GLExtensions()
{
const GLubyte *str;

    str = glGetString(GL_EXTENSIONS);
    GLString_Convert((char*)str);

#ifdef GLSL_WINDOWS
#ifdef USE_WGLEXT
    PFNWGLGETEXTENSIONSSTRINGARBPROC   wglGetExtensionsStringARB;
    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");

    if(wglGetExtensionsStringARB == NULL)
    {
       return;
    }

    str = (const GLubyte*) wglGetExtensionsStringARB(wglGetCurrentDC());
    GLString_Convert((char*)str);
#endif
#else
// ignore additional extensions....
#endif

}

//-----------------------------------------------------------------------------

GLExtensions::~GLExtensions()
{
 for (unsigned int i=0;i<ExtensionList.size();i++)
 {
    delete[] ExtensionList[i];  // free string
 }
}

//-----------------------------------------------------------------------------

void GLExtensions::GLString_Convert(char* str)
{
#define ELaAX_BUF_SIZE 80
char buf[ELaAX_BUF_SIZE];
int i=0;

   char* d = (char*) str;
        while (*d!=0)
        {
            if (*d==' ')
            {
                buf[i] = 0;
                i=0;
                addElement(buf);
            }
            else
            {
                buf[i++]= *d;
                if (i>ELaAX_BUF_SIZE-1) i=ELaAX_BUF_SIZE-1;
            }
            d++;
        }
}

//-----------------------------------------------------------------------------

void GLExtensions::addElement(char* str)
{
  char* s = new char[strlen(str)+1];
  strcpy(s, str);

  ExtensionList.push_back(s);

}

//-----------------------------------------------------------------------------

void GLExtensions::print(ostream& out)
{
   out << "OpenGL Vendor: " << (char*) glGetString(GL_VENDOR) << "\n";
   out << "OpenGL Renderer: " << (char*) glGetString(GL_RENDERER) << "\n";
   out << "OpenGL Version: " << (char*) glGetString(GL_VERSION) << "\n\n";

   for (unsigned int i=0;i<ExtensionList.size();i++)
   {
        out << ExtensionList[i] << endl;
   }

}

//-----------------------------------------------------------------------------

bool GLExtensions::check(char* extension_name)
{
   for (unsigned int i=0;i<ExtensionList.size();i++)
   {
        if (strstr((const char *)ExtensionList[i], extension_name) != 0)
        {
            return true;
        }
   }

   return false;
}

//-----------------------------------------------------------------------------

bool GLExtensions::init(void)
{
    init_ARB_extensions();
    return true;
}

//-----------------------------------------------------------------------------

// Bubble Sort to sort extension names.
void GLExtensions::sort(void)
{
char* tmp;

     for (unsigned int i = 0;  i < ExtensionList.size(); i++)
     {
        for (unsigned int j = (unsigned int) ExtensionList.size()-1; j>i; j--)
        {
            if (strcmp(ExtensionList[j-1],ExtensionList[j]) > 0)
            {
                tmp = ExtensionList[j-1];
                ExtensionList[j-1] = ExtensionList[j];
                ExtensionList[j] = tmp;
            }
        }
    }
}


// ************************************************************************
// Implementation der Klasse aShaderObject
// ************************************************************************


aShaderObject::aShaderObject()
{
  ShaderObject = 0;
  linker_log = 0;
  _mM = false;


  if (!initGLSL())
  {
    cout << "Error initializing OpenGL Shading Language function pointers" << endl;
  }

  if (!mcTestExtension("GL_ARB_shader_objects"))
        cout << "**warning** GL_ARB_shader_objects not defined!!\n";
  if (!mcTestExtension("GL_ARB_vertex_shader"))
        cout << "**warning** GL_ARB_vertex_shader not defined!!\n";
  if (!mcTestExtension("GL_ARB_fragment_shader"))
        cout << "**warning** GL_ARB_fragment_shader not defined!!\n";

  if (!mcTestExtension("GL_ARB_shading_language_100"))
        cout << "**warning** GL_ARB_shading_language_100 not defined!!\n";

  if (initGLSL())
  {
      ShaderObject = glCreateProgramObjectARB();
  }
  is_linked = false;
}

//-----------------------------------------------------------------------------

aShaderObject::~aShaderObject()
{
    if (linker_log!=0) free(linker_log);
    if (useGLSL)
    {
       for (unsigned int i=0;i<ShaderList.size();i++)
       {
            glDetachObjectARB(ShaderObject, ShaderList[i]->ProgramObject);
            CHECK_GL_ERROR(); // if you get an error here, you deleted the Program object first and then
                           // the ShaderObject! Always delete ShaderObjects last!
            if (_mM) delete ShaderList[i];
       }

       glDeleteObjectARB(ShaderObject);
       CHECK_GL_ERROR();
    }

}

//-----------------------------------------------------------------------------

bool aShaderObject::oglslEnabled(void)
{
   return useGLSL;
}

//-----------------------------------------------------------------------------

void aShaderObject::addShader(aShaderProgram* ShaderProgram)
{
if (!useGLSL) return;

   if (ShaderProgram==0) return;


   if (!ShaderProgram->is_compiled)
   {
        cout << "**warning** please compile program before adding object! trying to compile now...\n";
        if (!ShaderProgram->compile())
        {
            cout << "...compile ERROR!\n";
            return;
        }
        else
        {
            cout << "...ok!\n";
        }
   }

   ShaderList.push_back(ShaderProgram);

}

//-----------------------------------------------------------------------------

bool aShaderObject::link(void)
{
if (!useGLSL) return false;

unsigned int i;

    if (is_linked)  // already linked, detach everything first
    {
       cout << "**warning** Object is already linked, trying to link again" << endl;
       for (i=0;i<ShaderList.size();i++)
       {
            glDetachObjectARB(ShaderObject, ShaderList[i]->ProgramObject);
            CHECK_GL_ERROR();
       }
    }

    for (i=0;i<ShaderList.size();i++)
    {
        glAttachObjectARB(ShaderObject, ShaderList[i]->ProgramObject);
        CHECK_GL_ERROR();
        //cout << "attaching ProgramObj [" << i << "] @ 0x" << hex << ShaderList[i]->ProgramObject << " in ShaderObj @ 0x"  << ShaderObject << endl;
    }

    int linked;
    glLinkProgramARB(ShaderObject);
    CHECK_GL_ERROR();
    glGetObjectParameterivARB(ShaderObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
    CHECK_GL_ERROR();

    if (linked)
    {
        is_linked = true;
        return true;
    }
    else
    {
        cout << "**linker error**\n";
    }

return false;
}

//-----------------------------------------------------------------------------
// Compiler Log: Ausgabe der Compiler Meldungen in String

char* aShaderObject::getLinkerLog(void)
{
if (!useGLSL) return aGLSLErrorString[0];

 int blen = 0;
 int slen = 0;


 if (ShaderObject==0) return aGLSLErrorString[2];

 glGetObjectParameterivARB(ShaderObject, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
 CHECK_GL_ERROR();

 if (blen > 1)
 {
    if (linker_log!=0)
    {
        free(linker_log);
        linker_log =0;
    }
    if ((linker_log = (GLcharARB*)malloc(blen)) == NULL)
     {
        printf("ERROR: Could not allocate compiler_log buffer\n");
        return aGLSLErrorString[3];
    }

     glGetInfoLogARB(ShaderObject, blen, &slen, linker_log);
     CHECK_GL_ERROR();

 }
 if (linker_log!=0)
    return (char*) linker_log;

    return aGLSLErrorString[4];
}

void aShaderObject::begin(void)
{
if (!useGLSL) return;
if (ShaderObject == 0) return;
if (!_noshader) return;

    if (is_linked)
    {
        glUseProgramObjectARB(ShaderObject);
        CHECK_GL_ERROR();
    }
}

//-----------------------------------------------------------------------------

void aShaderObject::end(void)
{
if (!useGLSL) return;
if (!_noshader) return;


    glUseProgramObjectARB(0);
    CHECK_GL_ERROR();
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform1f(char* varname, GLfloat v0)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform1fARB(loc, v0);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform2f(char* varname, GLfloat v0, GLfloat v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform2fARB(loc, v0, v1);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform3f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform3fARB(loc, v0, v1, v2);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform4f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform4fARB(loc, v0, v1, v2, v3);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform1i(char* varname, GLint v0)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform1iARB(loc, v0);

    return true;
}
bool aShaderObject::sendUniform2i(char* varname, GLint v0, GLint v1)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform2iARB(loc, v0, v1);


    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform3i(char* varname, GLint v0, GLint v1, GLint v2)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform3iARB(loc, v0, v1, v2);

    return true;
}
bool aShaderObject::sendUniform4i(char* varname, GLint v0, GLint v1, GLint v2, GLint v3)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform4iARB(loc, v0, v1, v2, v3);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform1fv(char* varname, GLsizei count, GLfloat *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform1fvARB(loc, count, value);

    return true;
}
bool aShaderObject::sendUniform2fv(char* varname, GLsizei count, GLfloat *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform2fvARB(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform3fv(char* varname, GLsizei count, GLfloat *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform3fvARB(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform4fv(char* varname, GLsizei count, GLfloat *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform4fvARB(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform1iv(char* varname, GLsizei count, GLint *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform1ivARB(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform2iv(char* varname, GLsizei count, GLint *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform2ivARB(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform3iv(char* varname, GLsizei count, GLint *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform3ivARB(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniform4iv(char* varname, GLsizei count, GLint *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniform4ivARB(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniformMatrix2fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniformMatrix2fvARB(loc, count, transpose, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniformMatrix3fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniformMatrix3fvARB(loc, count, transpose, value);

    return true;
}

//-----------------------------------------------------------------------------

bool aShaderObject::sendUniformMatrix4fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_noshader) return true;

    GLint loc = GetUniLoc(varname);
    if (loc==-1) return false;  // can't find variable

    glUniformMatrix4fvARB(loc, count, transpose, value);

    return true;
}

//-----------------------------------------------------------------------------

GLint aShaderObject::GetUniLoc(const GLcharARB *name)
{
    GLint loc;

    loc = glGetUniformLocationARB(ShaderObject, name);
    if (loc == -1)
    {
        cout << "Error: can't find uniform variable \"" << name << "\"\n";
    }
    CHECK_GL_ERROR();
    return loc;
}

//-----------------------------------------------------------------------------

void aShaderObject::GetUniformfv(char* name, GLfloat* values)
{
if (!useGLSL) return;
    GLint loc;

    loc = glGetUniformLocationARB(ShaderObject, name);
    if (loc == -1)
    {
        cout << "Error: can't find uniform variable \"" << name << "\"\n";
    }
    glGetUniformfvARB(ShaderObject, loc, values);

}

//-----------------------------------------------------------------------------

void aShaderObject::GetUniformiv(char* name, GLint* values)
{
if (!useGLSL) return;


    GLint loc;

    loc = glGetUniformLocationARB(ShaderObject, name);
    if (loc == -1)
    {
        cout << "Error: can't find uniform variable \"" << name << "\"\n";
    }

    glGetUniformivARB(ShaderObject, loc, values);

}

bool  aShaderObject::_noshader = true;

// ************************************************************************
// Shader Program : Manage Shader Programs (Vertex/Fragment)
// ************************************************************************


aShaderProgram::aShaderProgram()
{
    initGLSL();
    compiler_log = 0;
    is_compiled = false;
    program_type = 0;
    ProgramObject = 0;
    ShaderSource = 0;
    _memalloc = false;

}

//-----------------------------------------------------------------------------

aShaderProgram::~aShaderProgram()
{
   if (compiler_log!=0) free(compiler_log);
   if (ShaderSource!=0)
   {
        if (_memalloc)
            delete[] ShaderSource;  // free ASCII Source
   }

   if (is_compiled)
   {
        glDeleteObjectARB(ProgramObject);
        CHECK_GL_ERROR();
   }
}

//-----------------------------------------------------------------------------
unsigned long getFileLength(ifstream& file)
{
    if(!file.good()) return 0;

    unsigned long pos=file.tellg();
    file.seekg(0,ios::end);
    unsigned long len = file.tellg();
    file.seekg(ios::beg);

    return len;
}


//-----------------------------------------------------------------------------

int aShaderProgram::load(char* filename)
{
    ifstream file;
    file.open(filename, ios::in);
    if(!file) return -1;

    unsigned long len = getFileLength(file);

    if (len==0) return -2;   // "Empty File"

    if (ShaderSource!=0)    // there is already a source loaded, free it!
    {
        if (_memalloc)
        delete[] ShaderSource;
    }

    ShaderSource = (GLubyte*) new char[len+1];
    if (ShaderSource == 0) return -3;   // can't reserve memory
    _memalloc = true;


    ShaderSource[len] = 0;  // len isn't always strlen cause some characters are stripped in ascii read...
                            // it is important to 0-terminate the real length later, len is just max possible value...

    unsigned int i=0;
    while (file.good())
    {
        ShaderSource[i++] = file.get();       // get character from file
        if (i>len) i=len;   // coding guidelines...
    }

    ShaderSource[i] = 0;  // 0 terminate it.

    file.close();

return 0;
}

//-----------------------------------------------------------------------------

void aShaderProgram::loadFromMemory(const char* program)
{
    if (ShaderSource!=0)    // there is already a source loaded, free it!
    {
        if (_memalloc)
        delete[] ShaderSource;
    }
   _memalloc = false;
   ShaderSource = (GLubyte*) program;

}


// ----------------------------------------------------
// Compiler Log: Ausgabe der Compiler Meldungen in String

char* aShaderProgram::getCompilerLog(void)
{
if (!useGLSL) return aGLSLErrorString[0];

 int blen = 0;
 int slen = 0;


 if (ProgramObject==0) return aGLSLErrorString[1];

 glGetObjectParameterivARB(ProgramObject, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
 CHECK_GL_ERROR();

 if (blen > 1)
 {
    if (compiler_log!=0)
    {
        free(compiler_log);
        compiler_log =0;
    }
    if ((compiler_log = (GLcharARB*)malloc(blen)) == NULL)
     {
        printf("ERROR: Could not allocate compiler_log buffer\n");
        return aGLSLErrorString[3];
    }

     glGetInfoLogARB(ProgramObject, blen, &slen, compiler_log);
     CHECK_GL_ERROR();
     //cout << "compiler_log: \n", compiler_log);
 }
 if (compiler_log!=0)
    return (char*) compiler_log;

    return aGLSLErrorString[4];
}

// ----------------------------------------------------

bool aShaderProgram::compile(void)
{
if (!useGLSL) return false;

is_compiled = false;

int compiled = 0;

  if (ShaderSource==0) return false;

  GLint    length = (GLint) strlen((const char*)ShaderSource);
  glShaderSourceARB(ProgramObject, 1, (const GLcharARB **)&ShaderSource, &length);
  CHECK_GL_ERROR();

  glCompileShaderARB(ProgramObject);
  CHECK_GL_ERROR();
  glGetObjectParameterivARB(ProgramObject, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
  CHECK_GL_ERROR();

  if (compiled) is_compiled=true;

return is_compiled;
}

// ----------------------------------------------------


aVertexShader::aVertexShader()
{
  program_type = 1;
   if (useGLSL)
   {
       ProgramObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
       CHECK_GL_ERROR();
   }
}

// ----------------------------------------------------

aVertexShader::~aVertexShader()
{
}

// ----------------------------------------------------

aFragmentShader::aFragmentShader()
{
    program_type = 2;
    if (useGLSL)
    {
        ProgramObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
        CHECK_GL_ERROR();
    }
}

// ----------------------------------------------------

aFragmentShader::~aFragmentShader()
{
}

// ----------------------------------------------------------------------------
// ShaderManager: Easy use of (multiple) Shaders

aShaderManager::aShaderManager()
{

}

aShaderManager::~aShaderManager()
{
   // free objects
   vector<aShaderObject*>::iterator  i=_shaderObjectList.begin();
   while (i!=_shaderObjectList.end())
   {
        aShaderObject* o = *i;
        i=_shaderObjectList.erase(i);
        delete o;
   }
}

// ----------------------------------------------------------------------------

aShaderObject* aShaderManager::loadfromFile(char* vertexFile, char* fragmentFile)
{
   aShaderObject* o = new aShaderObject();

   aVertexShader* tVertexShader = new aVertexShader;
   aFragmentShader* tFragmentShader = new aFragmentShader;

    // load vertex program
   if (vertexFile!=0)
   if (tVertexShader->load(vertexFile) != 0)
   {
     cout << "error: can't load vertex shader!\n";
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
   }

  // Load fragment program
  if (fragmentFile!=0)
  if (tFragmentShader->load(fragmentFile) != 0)
  {
     cout << "error: can't load fragment shader!\n";
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
  }

  // Compile vertex program
  if (vertexFile!=0)
  if (!tVertexShader->compile())
  {
      cout << "***COMPILER ERROR (Vertex Shader):\n";
      cout << tVertexShader->getCompilerLog() << endl;
      delete o;
      delete tVertexShader;
      delete tFragmentShader;
      return 0;
  }

  // Compile fragment program
  if (fragmentFile!=0)
  if (!tFragmentShader->compile())
  {
     cout << "***COMPILER ERROR (Fragment Shader):\n";
     cout << tFragmentShader->getCompilerLog() << endl;

     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;

  }

  // Add to object
  if (vertexFile!=0) o->addShader(tVertexShader);
  if (fragmentFile!=0) o->addShader(tFragmentShader);

  // link
  if (!o->link())
  {
     cout << "**LINKER ERROR\n";
     cout << o->getLinkerLog() << endl;
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
  }

  _shaderObjectList.push_back(o);
  o->manageMemory();

   return o;
}

// ----------------------------------------------------------------------------

aShaderObject* aShaderManager::loadfromMemory(const char* vertexMem, const char* fragmentMem)
{
  aShaderObject* o = new aShaderObject();

  aVertexShader* tVertexShader = new aVertexShader;
  aFragmentShader* tFragmentShader = new aFragmentShader;

  // get vertex program
  if (vertexMem!=0)
     tVertexShader->loadFromMemory(vertexMem);

  // get fragment program
  if (fragmentMem!=0)
     tFragmentShader->loadFromMemory(fragmentMem);

  // Compile vertex program
  if (vertexMem!=0)
  if (!tVertexShader->compile())
  {
      cout << "***COMPILER ERROR (Vertex Shader):\n";
      cout << tVertexShader->getCompilerLog() << endl;
      delete o;
      delete tVertexShader;
      delete tFragmentShader;
      return 0;
  }

  // Compile fragment program
  if (fragmentMem!=0)
  if (!tFragmentShader->compile())
  {
     cout << "***COMPILER ERROR (Fragment Shader):\n";
     cout << tFragmentShader->getCompilerLog() << endl;

     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;

  }

  // Add to object
  if (vertexMem!=0) o->addShader(tVertexShader);
  if (fragmentMem!=0) o->addShader(tFragmentShader);

  // link
  if (!o->link())
  {
     cout << "**LINKER ERROR\n";
     cout << o->getLinkerLog() << endl;
     delete o;
     delete tVertexShader;
     delete tFragmentShader;
     return 0;
  }

  _shaderObjectList.push_back(o);
  o->manageMemory();

   return o;
}

// ----------------------------------------------------------------------------

 bool  aShaderManager::free(aShaderObject* o)
 {
   vector<aShaderObject*>::iterator  i=_shaderObjectList.begin();
   while (i!=_shaderObjectList.end())
   {
        if ((*i)==o)
        {
            _shaderObjectList.erase(i);
            delete o;
            return true;
        }
        i++;
   }
   return false;
 }

// ----------------------------------------------------------------------------


