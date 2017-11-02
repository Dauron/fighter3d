#include "Shader.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include "../../Utils/Debug.h"
#include "../../Utils/Filesystem.h"

using namespace Graphics::OGL;

ShaderProgram *Shader::currShader       = NULL;

bool           Shader::FL_deferredMRT   = false;
GLuint         Shader::TexDiffuseImg    = 0;
GLuint         Shader::TexDiffuseUnit   = 0;
GLuint         Shader::TexBumpImg       = 0;
GLuint         Shader::TexBumpUnit      = 0;

GLenum         ShaderProgram::currProgram = 0;

ShaderProgram     Shader::spDeferred_Stage1_MRT;
ShaderTexBump     Shader::spDeferred_Stage1_MRT_TexBump;
ShaderDeferredS2  Shader::spDeferred_Stage2_Infinite;
ShaderDeferredS2  Shader::spDeferred_Stage2_Point;
ShaderDeferredS3  Shader::spDeferred_Stage3_Join;
ShaderConvolution Shader::spDeferred_Stage4_Convolution;
ShaderDeferredS4  Shader::spDeferred_Stage4_Tint;

ShaderProgram :: ShaderProgram()
{
    program            = 0;
    vertex_shader      = 0;
    fragment_shader    = 0;
    vertexShaderFile   = NULL;
    vertexShaderSrc    = NULL;
    fragmentShaderFile = NULL;
    fragmentShaderSrc  = NULL;
    FL_invalid         = true;
}

std::string LoadFile(std::string &fileName)
{
    char *outData = NULL;

    fileName = Filesystem::GetFullPath(fileName);
    std::ifstream in;
    in.open(fileName.c_str(), std::ios::in);
    if (in.is_open()) {
        in.seekg( 0, std::ios::end );
        size_t size = in.tellg();
        in.seekg( 0, std::ios::beg );

        outData = new char[size+1];
        in.read(outData, size);
        size = in.gcount();
        outData[size] = 0;
        in.close();
    }
    in.clear();

    if (outData)
    {
        std::string res(outData);
        delete[] outData;
        return res;
    }
    return std::string();
}

void AddIncludes(std::string &data, const std::string &dir)
{
    size_t      pos = 0;
    while ((pos = data.find("#include", pos)) != std::string::npos)
    {
        size_t fileS = data.find("\"", pos+9);
        size_t fileE = data.find("\"", fileS+1);

        if (fileS != std::string::npos && fileS != std::string::npos)
        {
            std::string relFileName = dir + "/" + data.substr(fileS+1, fileE-fileS-1);
            std::string inclData    = LoadFile (relFileName);
            AddIncludes(inclData, Filesystem::GetParentDir(relFileName));

            data = data.substr(0, pos) + inclData + data.substr(fileE+1);
        }
    }
}

void ShaderProgram :: Load(const char *vShaderFile, const char *fShaderFile)
{
    Unload();

    if (vShaderFile) {
        std::string fname = "Data/shaders/";
        fname += vShaderFile;
        std::string data = LoadFile (fname);
        AddIncludes(data, Filesystem::GetParentDir(fname));

        vertexShaderFile = strdup(fname.c_str());
        vertexShaderSrc  = strdup(data.c_str());
    }

    if (fShaderFile) {
        std::string fname = "Data/shaders/";
        fname += fShaderFile;
        std::string data = LoadFile (fname);
        AddIncludes(data, Filesystem::GetParentDir(fname));

        fragmentShaderFile = strdup(fname.c_str());
        fragmentShaderSrc  = strdup(data.c_str());
    }
}

void ShaderProgram :: Unload()
{
    if (vertexShaderFile)   delete[] vertexShaderFile;
    vertexShaderFile = NULL;
    if (vertexShaderSrc)    delete[] vertexShaderSrc;
    vertexShaderSrc = NULL;
    if (fragmentShaderFile) delete[] fragmentShaderFile;
    fragmentShaderFile = NULL;
    if (fragmentShaderSrc)  delete[] fragmentShaderSrc;
    fragmentShaderSrc = NULL;
}

GLenum ShaderProgram :: Create()
{
    assert(!program);
    assert(!vertex_shader);
    assert(!fragment_shader);

    FL_invalid = false;

    if (CheckForGLError("Pre create GLSL program")) {}

    if (!GLEW_ARB_shader_objects ||
        !GLEW_ARB_vertex_shader  ||
        !GLEW_ARB_fragment_shader)
    {
        if (!GLEW_ARB_shader_objects)  LOG(3, "WARNING: ARB_ShaderObjects is not supported\n");
        if (!GLEW_ARB_vertex_shader)   LOG(3, "WARNING: ARB_VertexShader is not supported\n");
        if (!GLEW_ARB_fragment_shader) LOG(3, "WARNING: ARB_FragmentShader is not supported\n");
        return 0;
    }

    if (!vertexShaderSrc && !fragmentShaderSrc)
    {
        LOG(3, "WARNING: shader source not found\n%s,%s\n", vertexShaderFile, fragmentShaderFile);
        return 0;
    }

    do {
        // Create Shader And Program Objects
        program = glCreateProgramObjectARB();
        if (CheckForGLError("Cannot create GLSL program")) {
            program = 0;
            break;
        }

        if (vertexShaderSrc) {
            vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
            if (CheckForGLError("Cannot create vertex shader")) {
                Destroy();
                FL_invalid = false;
                break;
            }
            glShaderSourceARB(vertex_shader, 1, (const char **)&vertexShaderSrc, NULL);
            glCompileShaderARB(vertex_shader);
            glAttachObjectARB(program, vertex_shader);
        }

        if (fragmentShaderSrc) {
            fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            if (CheckForGLError("Cannot create fragment shader")) {
                Destroy();
                FL_invalid = false;
                break;
            }
            glShaderSourceARB(fragment_shader, 1, (const char **)&fragmentShaderSrc, NULL);
            glCompileShaderARB(fragment_shader);
            glAttachObjectARB(program, fragment_shader);
        }
    }
    while(false);

    if (program) {
        // Link The Program Object
        glLinkProgramARB(program);

        int blen = 0;
        int slen = 0;
        glGetObjectParameterivARB(program, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);

        if (blen > 1)
        {
            char *linker_log;
            if ((linker_log = new GLcharARB[blen]) == NULL)
                DEB_LOG(0, "ERROR: Could not allocate compiler_log buffer\n");

            glGetInfoLogARB(program, blen, &slen, linker_log);
            linker_log[slen-1] = 0;

            if (strstr(linker_log, "error") || strstr(linker_log, "failed")
                || strstr(linker_log, "software"))
            {
                LOG(1, "%s,%s\n%s", vertexShaderFile, fragmentShaderFile, linker_log);
                delete[] linker_log;
                Destroy();
                FL_invalid = false;
                return 0;
            }
            if (!strstr(linker_log, "successful") && !strstr(linker_log, "linked"))
                LOG(2, "%s,%s\n%s", vertexShaderFile, fragmentShaderFile, linker_log);
            delete[] linker_log;
        }
    }

    return program;
}

void ShaderProgram :: Destroy()
{
    if (vertex_shader)
        glDeleteObjectARB(vertex_shader);
    if (fragment_shader)
        glDeleteObjectARB(fragment_shader);
    if (program)
        glDeleteObjectARB(program);
    Invalidate();
}

/*********** ShaderDeferredS2 ***********/
ShaderDeferredS2 :: ShaderDeferredS2() : ShaderProgram()
{
    uTEX_position = 0;
    uTEX_normal   = 0;
    uTEX_material = 0;
}

GLenum ShaderDeferredS2 :: Create()
{
    ShaderProgram::Create();
    if (program)
    {
        uTEX_position = glGetUniformLocationARB(program, "TEX_position");
        uTEX_normal   = glGetUniformLocationARB(program, "TEX_normal");
        uTEX_material = glGetUniformLocationARB(program, "TEX_material");
    }
    return program;
}

/*********** ShaderDeferredS3 ***********/
ShaderDeferredS3 :: ShaderDeferredS3() : ShaderProgram()
{
    uTEX_diffuse  = 0;
    uTEX_specular = 0;
}

GLenum ShaderDeferredS3 :: Create()
{
    ShaderProgram::Create();
    if (program)
    {
        uTEX_diffuse  = glGetUniformLocationARB(program, "TEX_diffuse");
        uTEX_specular = glGetUniformLocationARB(program, "TEX_specular");
    }
    return program;
}

/*********** ShaderDeferredS4 ***********/
ShaderDeferredS4 :: ShaderDeferredS4() : ShaderProgram()
{
    uTEX_image = 0;
}

GLenum ShaderDeferredS4 :: Create()
{
    ShaderProgram::Create();
    if (program)
        uTEX_image = glGetUniformLocationARB(program, "TEX_image");
    return program;
}

/*********** ShaderConvolution ***********/
ShaderConvolution :: ShaderConvolution() : ShaderDeferredS4()
{
    uTexOffset = 0;
    uMask      = 0;
}

GLenum ShaderConvolution :: Create()
{
    ShaderDeferredS4::Create();
    if (program)
    {
        uTexOffset = glGetUniformLocationARB(program, "TexOffset");
        uMask      = glGetUniformLocationARB(program, "Mask");
    }
    return program;
}

/*********** ShaderTexBump ***********/
ShaderTexBump :: ShaderTexBump() : ShaderProgram()
{
    uTEX_color = 0;
    uTEX_bump = 0;
}

GLenum ShaderTexBump :: Create()
{
    ShaderProgram::Create();
    if (program)
    {
        uTEX_color = glGetUniformLocationARB(program, "TEX_color");
        uTEX_bump  = glGetUniformLocationARB(program, "TEX_bump");
    }
    return program;
}
/*********** Shader ***********/
void Shader :: Load()
{
    spDeferred_Stage1_MRT.Load("Deferred_MRT.vert", "Deferred_MRT.frag");
    spDeferred_Stage1_MRT_TexBump.Load("Deferred_MRT_TexBump.vert", "Deferred_MRT_TexBump.frag");

    spDeferred_Stage2_Infinite.Load("PostProcessing.vert", "Deferred_Infinite.frag");
    spDeferred_Stage2_Point.Load("PostProcessing.vert", "Deferred_Point.frag");
    spDeferred_Stage3_Join.Load("PostProcessing.vert", "Deferred_Join.frag");

    spDeferred_Stage4_Convolution.Load("PostProcessing.vert", "ConvolutionFilter.frag");
    spDeferred_Stage4_Tint.Load("PostProcessing.vert", "Tint.frag");
}

void Shader :: Unload()
{
    spDeferred_Stage1_MRT.Unload();
    spDeferred_Stage1_MRT_TexBump.Unload();
    spDeferred_Stage2_Infinite.Unload();
    spDeferred_Stage2_Point.Unload();
    spDeferred_Stage3_Join.Unload();
    spDeferred_Stage4_Convolution.Unload();
    spDeferred_Stage4_Tint.Unload();
}

void Shader :: CreateS()
{
    spDeferred_Stage1_MRT.Create();
    spDeferred_Stage1_MRT_TexBump.Create();
    spDeferred_Stage2_Infinite.Create();
    spDeferred_Stage2_Point.Create();
    spDeferred_Stage3_Join.Create();
    spDeferred_Stage4_Convolution.Create();
    spDeferred_Stage4_Tint.Create();
}

void Shader :: DestroyS()
{
    spDeferred_Stage1_MRT.Destroy();
    spDeferred_Stage1_MRT_TexBump.Destroy();
    spDeferred_Stage2_Infinite.Destroy();
    spDeferred_Stage2_Point.Destroy();
    spDeferred_Stage3_Join.Destroy();
    spDeferred_Stage4_Convolution.Destroy();
    spDeferred_Stage4_Tint.Destroy();
}

void Shader :: Invalidate()
{
    spDeferred_Stage1_MRT.Invalidate();
    spDeferred_Stage1_MRT_TexBump.Invalidate();
    spDeferred_Stage2_Infinite.Invalidate();
    spDeferred_Stage2_Point.Invalidate();
    spDeferred_Stage3_Join.Invalidate();
    spDeferred_Stage4_Convolution.Invalidate();
    spDeferred_Stage4_Tint.Invalidate();
}

bool Shader :: StartDeferred(xDeferredProgramStage stage)
{
    FL_deferredMRT = false;
    switch(stage)
    {
        case xDPS_Stage1_MRT:
            currShader = &spDeferred_Stage1_MRT;
            FL_deferredMRT = true;
            break;
        case xDPS_Stage2_Infinite:
            currShader = &spDeferred_Stage2_Infinite;
            break;
        case xDPS_Stage2_Point:
            currShader = &spDeferred_Stage2_Point;
            break;
        case xDPS_Stage3_Join:
            currShader = &spDeferred_Stage3_Join;
            break;
        case xDPS_Stage4_Convolution:
            currShader = &spDeferred_Stage4_Convolution;
            break;
        case xDPS_Stage4_Tint:
            currShader = &spDeferred_Stage4_Tint;
            break;
    }
    if (!currShader) return false;

    if (currShader->FL_invalid) currShader->Create();

    if (ShaderProgram::currProgram != currShader->program)
        glUseProgramObjectARB(ShaderProgram::currProgram = currShader->program);

    return currShader->program;
}

void Shader :: StopDeferred()
{
    currShader = NULL;
    if (ShaderProgram::currProgram)
        glUseProgramObjectARB(ShaderProgram::currProgram = 0);
    FL_deferredMRT = false;
}

bool Shader :: Start()
{
    //assert(program);
    //assert(!IsCurrent());
    // Use The Program Object Instead Of Fixed Function OpenGL
    //assert(shaderState == xState_Disable || shaderState == xState_Enable || shaderState == xState_Off);

    if (!FL_deferredMRT) return false;

    currShader = &spDeferred_Stage1_MRT;
    if (TexDiffuseImg && TexBumpImg)
        currShader = &spDeferred_Stage1_MRT_TexBump;

    if (currShader->FL_invalid) currShader->Create();

    if (ShaderProgram::currProgram != currShader->program)
    {
        glUseProgramObjectARB(ShaderProgram::currProgram = currShader->program);
        if (TexDiffuseImg && TexBumpImg)
        {
            ShaderTexBump *currShader = (ShaderTexBump *)Shader::currShader;
            glUniform1iARB( currShader->uTEX_color, TexDiffuseUnit);
            glUniform1iARB( currShader->uTEX_bump,  TexBumpUnit);
        }
    }
    return currShader->program;
}

void Shader :: Suspend()
{
    //assert(program);
    //assert(IsCurrent());
    // Use The Fixed Function OpenGL
    if (!FL_deferredMRT) return;
    if (ShaderProgram::currProgram)
        glUseProgramObjectARB(ShaderProgram::currProgram = 0);
    currShader = NULL;
}
