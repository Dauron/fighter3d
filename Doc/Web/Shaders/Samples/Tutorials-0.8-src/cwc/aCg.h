// ================================================
// Support for NVidia CG Programs
// Adds support for Nvidia CG Programs
// (2 profiles)
//
// This is just a prototype and doesn't work yet.
//
// (c) 2004 by Martin Christen. All Rights Reserved
// ================================================

#include "aGLSL.h"
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <iostream>
#include <vector>

//-----------------------------------------------------------------------------

class aShaderProgramCg
{
public:
    virtual int load(char* filename) = 0;  //!< read file, if result is 0 everything is ok. -1: File not found, -2: Empty File, -3: no memory, -9: This machine doesn't support CG

    virtual bool compile(void) = 0;   //!< compile program (currently load function compiles too)

    virtual char* getCompilerLog(void) = 0;   //!< get compiler messages (currently not working)


private:
        CGcontext   _Context;
        CGprogram   _Program;
        CGprogram   _KdParam;

        // Benutzerfedinierte Parameter
        std::vector<CGparameter> _ParamList;

        CGprofile   _profile;

};

//-----------------------------------------------------------------------------

class aVertexShaderCg : public aShaderProgramCg
{
  public:
    aVertexShaderCg()
    {    _Program = 0;
    }


    ~aVertexShaderCg()

    int load(char* filename)
    {
        if (cgGLIsProfileSupported(CG_PROFILE_VP30))
        {
            _profile = CG_PROFILE_VP30;
        }
        else
        {

            if (cgGLIsProfileSupported(CG_PROFILE_ARBVP1))
            {
            _profile = CG_PROFILE_ARBVP1;
            }
            else
            {
                return -9;
            }
        }

        _Context = cgCreateContext();
        _Program = cgCreateProgramFromFile(_Context,CG_SOURCE, filename, _profile, NULL, NULL);

         cgGLLoadProgram(_Program);
         //_ModelViewProjParam = cgGetNamedParameter(_Program, "ModelViewProj");
         return 0;
    }

};

//-----------------------------------------------------------------------------

class aFragmentShaderCg : public aShaderProgramCg
{
  public:
       aFragmentShaderCg();
       ~aFragmentShaderCg();


  private:
};

//-----------------------------------------------------------------------------

class aShaderObjectCg
{
  public:
        aShaderObjectCg();
        ~aShaderObjectCg();

        void addShader(aFragmentShaderCg* ShaderProgram); // add Fragment Shader
        void addShader(aVertexShaderCg* ShaderProgram);   // add Vertex Shader

        sendUniform1f(char* varname, float value);
        sendUniform2f(char* varname, float v1, float v2);
        sendUniform3f(char* varname, float v1, float v2, float v3);
        sendUniform4f(char* varname, float v1, float v2, float v3, float v4);


  private:
    aFragmentShaderCg* _F;
    aVertexShaderCg*   _V;

};

//-----------------------------------------------------------------------------

