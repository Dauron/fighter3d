#include "GLShader.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include "../Utils/Debug.h"
#include "../Utils/Filesystem.h"

ShaderProgram *GLShader::currShader       = NULL;
xLightType     GLShader::lightType        = xLight_NONE;
xState         GLShader::shaderState      = xState_Enable;
xState         GLShader::textureState     = xState_Enable;
xState         GLShader::skeletalState    = xState_Enable;
bool           GLShader::ambient          = false;
bool           GLShader::diffuse          = false;
bool           GLShader::specular         = false;
GLenum         ShaderProgram::currProgram = 0;

ShaderLighting GLShader::slNoLighting;
ShaderLighting GLShader::slGlobalA;
ShaderLighting GLShader::slInfiniteA;
ShaderLighting GLShader::slInfiniteDS;
ShaderLighting GLShader::slInfiniteADS;
ShaderLighting GLShader::slPointA;
ShaderLighting GLShader::slPointDS;
ShaderLighting GLShader::slPointADS;
ShaderLighting GLShader::slSpotA;
ShaderLighting GLShader::slSpotDS;
ShaderLighting GLShader::slSpotADS;

ShaderProgram :: ShaderProgram()
{
    program            = 0;
    vertex_shader      = 0;
    fragment_shader    = 0;
    vertexShaderFile   = NULL;
    vertexShaderSrc    = NULL;
    fragmentShaderFile = NULL;
    fragmentShaderSrc  = NULL;
}

void ShaderProgram :: Load(const char *vShaderFile, const char *fShaderFile)
{
    Unload();

    std::ifstream in;

    if (vShaderFile) {
        std::string fname = vShaderFile;
        fname = Filesystem::GetFullPath("Data/shaders/" + fname);
        vertexShaderFile = strdup(fname.c_str());
        in.open(vertexShaderFile, std::ios::in);
        if (in.is_open()) {
            in.seekg( 0, std::ios::end );
            int width = in.tellg();
            in.seekg( 0, std::ios::beg );

            vertexShaderSrc = new char[width+1];
            in.read(vertexShaderSrc, width);
            width = in.gcount();
            vertexShaderSrc[width] = 0;
            in.close();
        }
        in.clear();
    }

    if (fShaderFile) {
        std::string fname = fShaderFile;
        fname = Filesystem::GetFullPath("Data/shaders/" + fname);
        fragmentShaderFile = strdup(fname.c_str());
        in.open(fragmentShaderFile, std::ios::in);
        if (in.is_open()) {
            in.seekg( 0, std::ios::end );
            int width = in.tellg();
            in.seekg( 0, std::ios::beg );

            fragmentShaderSrc = new char[width+1];
            in.read(fragmentShaderSrc, width);
            width = in.gcount();
            fragmentShaderSrc[width] = 0;
            in.close();
        }
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

GLenum ShaderProgram :: Initialize()
{
    assert(!program);
    assert(!vertex_shader);
    assert(!fragment_shader);
    if (CheckForGLError("Pre create GLSL program")) {}

    if (!GLExtensions::Exists_ARB_ShaderObjects ||
        !GLExtensions::Exists_ARB_VertexShader  ||
        !GLExtensions::Exists_ARB_FragmentShader)
    {
        if (!GLExtensions::Exists_ARB_ShaderObjects)  LOG(3, "WARNING: ARB_ShaderObjects is not supported\n");
        if (!GLExtensions::Exists_ARB_VertexShader)   LOG(3, "WARNING: ARB_VertexShader is not supported\n");
        if (!GLExtensions::Exists_ARB_FragmentShader) LOG(3, "WARNING: ARB_FragmentShader is not supported\n");
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
                Terminate();
                break;
            }
            glShaderSourceARB(vertex_shader, 1, (const char **)&vertexShaderSrc, NULL);
            glCompileShaderARB(vertex_shader);
            glAttachObjectARB(program, vertex_shader);
        }

        if (fragmentShaderSrc) {
            fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            if (CheckForGLError("Cannot create fragment shader")) {
                Terminate();
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
                Terminate();
                return 0;
            }
            if (!strstr(linker_log, "successful"))
                LOG(2, "%s,%s\n%s", vertexShaderFile, fragmentShaderFile, linker_log);
            delete[] linker_log;
        }
    }

    return program;
}

void ShaderProgram :: Terminate()
{
    if (vertex_shader)
        glDeleteObjectARB(vertex_shader);
    if (fragment_shader)
        glDeleteObjectARB(fragment_shader);
    if (program)
        glDeleteObjectARB(program);
    Invalidate();
}

ShaderSkeletal :: ShaderSkeletal() : ShaderProgram()
{
    uBones        = 0;
    aBoneIdxWghts = 0;
}

GLenum ShaderSkeletal :: Initialize()
{
    ShaderProgram::Initialize();
    if (program)
    {
        uBones        = glGetUniformLocationARB(program, "bones");
        aBoneIdxWghts = glGetAttribLocationARB (program, "boneIdxWghts");
    }
    return program;
}

void ShaderLighting :: Initialize()
{
    Plain.Initialize();
    Textured.Initialize();
    PlainSkeletal.Initialize();
    TexturedSkeletal.Initialize();
}

void ShaderLighting :: Terminate()
{
    Plain.Terminate();
    Textured.Terminate();
    PlainSkeletal.Terminate();
    TexturedSkeletal.Terminate();
}

void ShaderLighting :: Invalidate()
{
    Plain.Invalidate();
    Textured.Invalidate();
    PlainSkeletal.Invalidate();
    TexturedSkeletal.Invalidate();
}

void GLShader :: Load()
{
    slNoLighting.Plain.Load("NoLights_noTex.vert", "NoLights_noTex.frag");
    slNoLighting.Textured.Load("NoLights_Tex.vert", "NoLights_Tex.frag");
    slNoLighting.PlainSkeletal.Load("NoLights_noTex_Skel.vert", "NoLights_noTex.frag");
    slNoLighting.TexturedSkeletal.Load("NoLights_Tex_Skel.vert", "NoLights_Tex.frag");

    slGlobalA.Plain.Load("Global_A_noTex.vert", "Global_A_noTex.frag");
    slGlobalA.Textured.Load("Global_A_Tex.vert", "Global_A_Tex.frag");
    slGlobalA.PlainSkeletal.Load("Global_A_noTex_Skel.vert", "Global_A_noTex.frag");
    slGlobalA.TexturedSkeletal.Load("Global_A_Tex_Skel.vert", "Global_A_Tex.frag");

    slInfiniteA.Plain.Load("Infinite_A_noTex.vert", "Infinite_A_noTex.frag");
    slInfiniteA.Textured.Load("Infinite_A_Tex.vert", "Infinite_A_Tex.frag");
    slInfiniteA.PlainSkeletal.Load("Infinite_A_noTex_Skel.vert", "Infinite_A_noTex.frag");
    slInfiniteA.TexturedSkeletal.Load("Infinite_A_Tex_Skel.vert", "Infinite_A_Tex.frag");

    slInfiniteDS.Plain.Load("Infinite_DS_noTex.vert", "Infinite_DS_noTex.frag");
    slInfiniteDS.Textured.Load("Infinite_DS_Tex.vert", "Infinite_DS_Tex.frag");
    slInfiniteDS.PlainSkeletal.Load("Infinite_DS_noTex_Skel.vert", "Infinite_DS_noTex.frag");
    slInfiniteDS.TexturedSkeletal.Load("Infinite_DS_Tex_Skel.vert", "Infinite_DS_Tex.frag");

    slInfiniteADS.Plain.Load("Infinite_DS_noTex.vert", "Infinite_ADS_noTex.frag");
    slInfiniteADS.Textured.Load("Infinite_DS_Tex.vert", "Infinite_ADS_Tex.frag");
    slInfiniteADS.PlainSkeletal.Load("Infinite_DS_noTex_Skel.vert", "Infinite_ADS_noTex.frag");
    slInfiniteADS.TexturedSkeletal.Load("Infinite_DS_Tex_Skel.vert", "Infinite_ADS_Tex.frag");

    slPointA.Plain.Load("Point_A_noTex.vert", "Point_A_noTex.frag");
    slPointA.Textured.Load("Point_A_Tex.vert", "Point_A_Tex.frag");
    slPointA.PlainSkeletal.Load("Point_A_noTex_Skel.vert", "Point_A_noTex.frag");
    slPointA.TexturedSkeletal.Load("Point_A_Tex_Skel.vert", "Point_A_Tex.frag");

    slPointDS.Plain.Load("Point_DS_noTex.vert", "Point_DS_noTex.frag");
    slPointDS.Textured.Load("Point_DS_Tex.vert", "Point_DS_Tex.frag");
    slPointDS.PlainSkeletal.Load("Point_DS_noTex_Skel.vert", "Point_DS_noTex.frag");
    slPointDS.TexturedSkeletal.Load("Point_DS_Tex_Skel.vert", "Point_DS_Tex.frag");

    slPointADS.Plain.Load("Point_DS_noTex.vert", "Point_ADS_noTex.frag");
    slPointADS.Textured.Load("Point_DS_Tex.vert", "Point_ADS_Tex.frag");
    //slPointADS.PlainSkeletal.Load("Point_DS_noTex_Skel.vert", "Point_ADS_noTex.frag");
    // ATI compiler workaround
    slPointADS.PlainSkeletal.Load("Point_DS_Tex_Skel.vert", "Point_ADS_noTex.frag");
    slPointADS.TexturedSkeletal.Load("Point_DS_Tex_Skel.vert", "Point_ADS_Tex.frag");

    slSpotA.Plain.Load("Spot_A_noTex.vert", "Spot_A_noTex.frag");
    slSpotA.Textured.Load("Spot_A_Tex.vert", "Spot_A_Tex.frag");
    slSpotA.PlainSkeletal.Load("Spot_A_noTex_Skel.vert", "Spot_A_noTex.frag");
    slSpotA.TexturedSkeletal.Load("Spot_A_Tex_Skel.vert", "Spot_A_Tex.frag");

    slSpotDS.Plain.Load("Spot_DS_noTex.vert", "Spot_DS_noTex.frag");
    slSpotDS.Textured.Load("Spot_DS_Tex.vert", "Spot_DS_Tex.frag");
    slSpotDS.PlainSkeletal.Load("Spot_DS_noTex_Skel.vert", "Spot_DS_noTex.frag");
    slSpotDS.TexturedSkeletal.Load("Spot_DS_Tex_Skel.vert", "Spot_DS_Tex.frag");

    slSpotADS.Plain.Load("Spot_DS_noTex.vert", "Spot_ADS_noTex.frag");
    slSpotADS.Textured.Load("Spot_DS_Tex.vert", "Spot_ADS_Tex.frag");
    slSpotADS.PlainSkeletal.Load("Spot_DS_noTex_Skel.vert", "Spot_ADS_noTex.frag");
    slSpotADS.TexturedSkeletal.Load("Spot_DS_Tex_Skel.vert", "Spot_ADS_Tex.frag");
}

void GLShader :: Unload()
{
    slNoLighting.Plain.Unload();
    slNoLighting.Textured.Unload();
    slNoLighting.PlainSkeletal.Unload();
    slNoLighting.TexturedSkeletal.Unload();

    slGlobalA.Plain.Unload();
    slGlobalA.Textured.Unload();
    slGlobalA.PlainSkeletal.Unload();
    slGlobalA.TexturedSkeletal.Unload();

    slInfiniteA.Plain.Unload();
    slInfiniteA.Textured.Unload();
    slInfiniteA.PlainSkeletal.Unload();
    slInfiniteA.TexturedSkeletal.Unload();

    slInfiniteDS.Plain.Unload();
    slInfiniteDS.Textured.Unload();
    slInfiniteDS.PlainSkeletal.Unload();
    slInfiniteDS.TexturedSkeletal.Unload();

    slInfiniteADS.Plain.Unload();
    slInfiniteADS.Textured.Unload();
    slInfiniteADS.PlainSkeletal.Unload();
    slInfiniteADS.TexturedSkeletal.Unload();

    slPointA.Plain.Unload();
    slPointA.Textured.Unload();
    slPointA.PlainSkeletal.Unload();
    slPointA.TexturedSkeletal.Unload();

    slPointDS.Plain.Unload();
    slPointDS.Textured.Unload();
    slPointDS.PlainSkeletal.Unload();
    slPointDS.TexturedSkeletal.Unload();

    slPointADS.Plain.Unload();
    slPointADS.Textured.Unload();
    slPointADS.PlainSkeletal.Unload();
    slPointADS.TexturedSkeletal.Unload();

    slSpotA.Plain.Unload();
    slSpotA.Textured.Unload();
    slSpotA.PlainSkeletal.Unload();
    slSpotA.TexturedSkeletal.Unload();

    slSpotDS.Plain.Unload();
    slSpotDS.Textured.Unload();
    slSpotDS.PlainSkeletal.Unload();
    slSpotDS.TexturedSkeletal.Unload();

    slSpotADS.Plain.Unload();
    slSpotADS.Textured.Unload();
    slSpotADS.PlainSkeletal.Unload();
    slSpotADS.TexturedSkeletal.Unload();
}

void GLShader :: Initialize()
{
    slNoLighting.Initialize();
    slGlobalA.Initialize();
    slInfiniteA.Initialize();
    slInfiniteDS.Initialize();
    slInfiniteADS.Initialize();
    slPointA.Initialize();
    slPointDS.Initialize();
    slPointADS.Initialize();
    slSpotA.Initialize();
    slSpotDS.Initialize();
    slSpotADS.Initialize();
}

void GLShader :: Terminate()
{
    slNoLighting.Terminate();
    slGlobalA.Terminate();
    slInfiniteA.Terminate();
    slInfiniteDS.Terminate();
    slInfiniteADS.Terminate();
    slPointA.Terminate();
    slPointDS.Terminate();
    slPointADS.Terminate();
    slSpotA.Terminate();
    slSpotDS.Terminate();
    slSpotADS.Terminate();
}

void GLShader :: Invalidate()
{
    slNoLighting.Invalidate();
    slGlobalA.Invalidate();
    slInfiniteA.Invalidate();
    slInfiniteDS.Invalidate();
    slInfiniteADS.Invalidate();
    slPointA.Invalidate();
    slPointDS.Invalidate();
    slPointADS.Invalidate();
    slSpotA.Invalidate();
    slSpotDS.Invalidate();
    slSpotADS.Invalidate();
}

bool GLShader :: Start()
{
    //assert(program);
    //assert(!IsCurrent());
    // Use The Program Object Instead Of Fixed Function OpenGL
    //assert(shaderState == xState_Disable || shaderState == xState_Enable || shaderState == xState_Off);

    if (shaderState == xState_Disable) return false;
    if (!Config::EnableShaders) return false;
    //if (shaderState != xState_Enable || shaderState == xState_Off) return false;
    shaderState = xState_On;

    ShaderLighting *slShader;

    switch (lightType)
    {
        case xLight_NONE:
            slShader = &slNoLighting;
            break;
        case xLight_GLOBAL:
            slShader = &slGlobalA;
            break;
        case xLight_INFINITE:
            if (ambient && !diffuse && !specular)
                slShader = &slInfiniteA;
            else
            if (!ambient && diffuse && specular)
                slShader = &slInfiniteDS;
            else
            if (ambient && diffuse && specular)
                slShader = &slInfiniteADS;
            else
                slShader = NULL;
            break;
        case xLight_POINT:
            if (ambient && !diffuse && !specular)
                slShader = &slPointA;
            else
            if (!ambient && diffuse && specular)
                slShader = &slPointDS;
            else
            if (ambient && diffuse && specular)
                slShader = &slPointADS;
            else
                slShader = NULL;
            break;
        case xLight_SPOT:
            if (ambient && !diffuse && !specular)
                slShader = &slSpotA;
            else
            if (!ambient && diffuse && specular)
                slShader = &slSpotDS;
            else
            if (ambient && diffuse && specular)
                slShader = &slSpotADS;
            else
                slShader = NULL;
            break;
        default:
            slShader = NULL;
            break;
    }
    if (!slShader) return false;

    if (textureState == xState_On && skeletalState == xState_On)
        currShader = & slShader->TexturedSkeletal;
    else
    if (textureState == xState_On)
        currShader = & slShader->Textured;
    else
    if (skeletalState == xState_On)
        currShader = & slShader->PlainSkeletal;
    else
        currShader = & slShader->Plain;

    if (!currShader->program) return false;

    if (ShaderProgram::currProgram != currShader->program)
        glUseProgramObjectARB(ShaderProgram::currProgram = currShader->program);
    return true;
}

void GLShader :: Suspend()
{
    //assert(program);
    //assert(IsCurrent());
    // Use The Fixed Function OpenGL
    if (shaderState == xState_Disable) return;
    shaderState = xState_Enable;
    if (ShaderProgram::currProgram)
        glUseProgramObjectARB(ShaderProgram::currProgram = 0);
    currShader = NULL;
}
