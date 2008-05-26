/******************************************************************************
aGLSL.h
Definitions and helper classes for OGLSL

*******************************************************
OGLSL C++ Framework 0.7.0 beta

report bugs: christen@clockworkcoders.com
*******************************************************

Beta 0.7.0:
    * Better Error String management
    * added aShaderProgram::loadFromMemory(char* program) to load a program
      from a char array.
    * Added a high level Interface to load a simple vertex/fragment shader setup:
      "aShaderManager"


(c) 2003,2004 by Martin Christen. All Rights reserved.
    christen@clockworkcoders.com
******************************************************************************/
#ifndef A_GLSL_H
#define A_GLSL_H

// Operating System
#if defined(__MACOSX__)
#define GLSL_MACOSX
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define GLSL_WINDOWS
#include <windows.h>
//#define USE_WGLEXT          // if you dont want WGL Support, just remove this define
#ifdef USE_WGLEXT
#include <GL/gl.h>
#include <GL/wglext.h>
#endif
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define GLSL_LINUX
#else
#error "sorry, your OS isn't supported (yet)!"
#endif

#include "aGL_Extensions.h"


#include <GL/gl.h>
#include <vector>
#include <iostream>




//----------------------------------------------------------------
// hopefully the following error messages will disappear soon!
// I need working GLSL implementations on those OSes to test it...
#ifdef GLSL_MACOSX
#error "sorry, MacOSX port doesn't work yet"
#endif
#ifdef GLSL_LINUX
#error "sorry, Linux port probably doesn't work yet..."
#endif
//----------------------------------------------------------------


// useful Macros:

#ifdef GLSL_WINDOWS
#define aLoadExtension(functype,funcname) ((funcname = (functype) wglGetProcAddress( #funcname )) == NULL)
#endif

#ifdef GLSL_LINUX
#define aLoadExtension(functype,funcname) ((funcname = (functype) glXGetProcAddressARB( #funcname )) == NULL)
#endif

// useful helper functions:
bool mcTestExtension(const char* extension_name);



// ***************************
// GLExtensions - Helper Class
// ***************************

class GLExtensions
{
public:
    GLExtensions();
    ~GLExtensions();

    void print(std::ostream& out=std::cout); //!< output list to ostream, standard: console
    bool check(char* extension_name);        //!< returns true if extension exists...

    void sort(void);                        //!< sort extensions (alphabetical)

    bool init(void);                        //!< init all extensions

    std::vector<char*>   ExtensionList;     //!< List of all available OpenGL Extensions

private:
    void GLString_Convert(char* str);
    void addElement(char* str);

};

//-----------------------------------------------------------------------------

class aShaderProgram
{
    friend class aShaderObject;

public:
    aShaderProgram();
    ~aShaderProgram();

    int load(char* filename);   //!< read file, if result is 0 everything is ok. -1: File not found, -2: Empty File, -3: no memory
    void loadFromMemory(const char* program); //!< load program from char array, make sure program is 0 terminated!


    bool compile(void);         //!< compile program

    char* getCompilerLog(void);  //!< get compiler messages

protected:

    int                 program_type;          //!< 1=Vertex Program, 2=Fragment Program, 0=none

    GLhandleARB         ProgramObject;         //!< Program Object
    GLubyte*            ShaderSource;          //!< ASCII Source-Code

    GLcharARB*          compiler_log;

    bool                is_compiled;            //!< true if compiled
    bool                _memalloc;               //!< true if shader allocated memory


};

// --------------------------------------------------------------

class aVertexShader : public aShaderProgram
{
  public:
       aVertexShader();
       ~aVertexShader();
};

// --------------------------------------------------------------

class aFragmentShader : public aShaderProgram
{
 public:
    aFragmentShader();
    ~aFragmentShader();

};

//-----------------------------------------------------------------------------

class aShaderObject
{
public:
    aShaderObject();            // Standard Constructor
    ~aShaderObject();           // Destructor

    void addShader(aShaderProgram* ShaderProgram); //!< add a Vertex or Fragment Program

    bool link(void);            //!< Link all Shaders
    char* getLinkerLog(void);   //!< get Linker messages

    void begin();    //!< use Shader. OpenGL calls will go through shader.
    void end();        //!< Stop using this shader. OpenGL calls will go through regular pipeline.

    bool oglslEnabled(void);    //!< returns true if OGLSL is enabled. It is possible user hardware doesn't support OGLSL!

    // Send Variables to Program

    bool sendUniform1f(char* varname, GLfloat v0); //!< send float to program
    bool sendUniform2f(char* varname, GLfloat v0, GLfloat v1); //!< send vec2 to program
    bool sendUniform3f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2); //!< send vec3 to program
    bool sendUniform4f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3); //!< send vec4 to program

    bool sendUniform1i(char* varname, GLint v0);
    bool sendUniform2i(char* varname, GLint v0, GLint v1);
    bool sendUniform3i(char* varname, GLint v0, GLint v1, GLint v2);
    bool sendUniform4i(char* varname, GLint v0, GLint v1, GLint v2, GLint v3);

    bool sendUniform1fv(char* varname, GLsizei count, GLfloat *value);
    bool sendUniform2fv(char* varname, GLsizei count, GLfloat *value);
    bool sendUniform3fv(char* varname, GLsizei count, GLfloat *value);
    bool sendUniform4fv(char* varname, GLsizei count, GLfloat *value);

    bool sendUniform1iv(char* varname, GLsizei count, GLint *value);
    bool sendUniform2iv(char* varname, GLsizei count, GLint *value);
    bool sendUniform3iv(char* varname, GLsizei count, GLint *value);
    bool sendUniform4iv(char* varname, GLsizei count, GLint *value);

    bool sendUniformMatrix2fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value);
    bool sendUniformMatrix3fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value);
    bool sendUniformMatrix4fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value);


    // Receive Uniform variables:
    void GetUniformfv(char* name, GLfloat* values);
    void GetUniformiv(char* name, GLint* values);


    void manageMemory(void){_mM = true;}

    // Turn off all Shaders:
    static void useShader(bool b)        //!< Deactivate Shader
    {
        _noshader = b;
    }

private:
    GLint GetUniLoc(const GLcharARB *name);      // get location of a variable

    GLhandleARB         ShaderObject;            // Shader Object


    GLcharARB*          linker_log;
    bool                is_linked;
    std::vector<aShaderProgram*> ShaderList;     // List of all Shader Programs

    bool                _mM;
    static bool         _noshader;

};

//-----------------------------------------------------------------------------
// To simplify the process loading/compiling/linking shaders I created this
// high level interface to setup a vertex/fragment shader.

class aShaderManager
{
public:
    aShaderManager();
    ~aShaderManager();

    aShaderObject* loadfromFile(char* vertexFile, char* fragmentFile);    // load vertex/fragment shader from file
    aShaderObject* loadfromMemory(const char* vertexMem, const char* fragmentMem);

    //aShaderObject* arb_loadfromFile(char* vertexFile, char* fragmentFile);
    //aShaderObject* arb_loadfromMemory(const char* vertexMem, const char* fragmentMem);


    bool           free(aShaderObject* o);

private:
    std::vector<aShaderObject*>  _shaderObjectList;
};


#endif // A_GLSL_H