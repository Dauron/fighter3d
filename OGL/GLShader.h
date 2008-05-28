#ifndef __incl_GLShader_h
#define __incl_GLShader_h     1

#include "Extensions/ARB_vertex_shader.h"
#include "Extensions/ARB_fragment_shader.h"
#include "../Math/xMath.h"
#include "../Math/xLight.h"

struct ShaderProgram
{
    static GLenum currProgram;

    GLenum program;

    GLenum vertex_shader;
    char *vertexShaderFile;
    char *vertexShaderSrc;

    GLenum fragment_shader;
    char *fragmentShaderFile;
    char *fragmentShaderSrc;

    ShaderProgram();
    virtual ~ShaderProgram() {}

    void Load(const char *vShaderFile, const char *fShaderFile);
    void Unload();

    GLenum IsInitialized() { return program; }
    bool IsCurrent() { return program == currProgram; }

    virtual GLenum Initialize();
    virtual void   Terminate();
    virtual void   Invalidate()
    {
        program = vertex_shader = fragment_shader = 0;
    }
};

struct ShaderSkeletal : public ShaderProgram
{
    // identifiers of vertex uniforms and attributes
    int uBones;
    int aBoneIdxWghts;

    ShaderSkeletal();
    virtual ~ShaderSkeletal() {}

    virtual GLenum Initialize();
};

struct ShaderLighting
{
    ShaderProgram  Plain;
    ShaderProgram  Textured;
    ShaderSkeletal PlainSkeletal;
    ShaderSkeletal TexturedSkeletal;

    void Initialize();
    void Terminate();
    void Invalidate();
};

enum xState
{
    xState_Disable = -2,
    xState_Enable  = -1,
    xState_Off     =  0,
    xState_On      =  1
};

class GLShader
{
    static xLightType     lightType;

    static xState         shaderState;   // can we use gl shaders ?
    static xState         textureState;  // can we use textures ?
    static xState         skeletalState; // can we use skeleton ?
    static bool ambient;
    static bool diffuse;
    static bool specular;

    static ShaderLighting slNoLighting;
    static ShaderLighting slGlobalA;
    static ShaderLighting slInfiniteA;
    static ShaderLighting slInfiniteDS;
    static ShaderLighting slInfiniteADS;
    static ShaderLighting slPointA;
    static ShaderLighting slPointDS;
    static ShaderLighting slPointADS;
    static ShaderLighting slSpotA;
    static ShaderLighting slSpotDS;
    static ShaderLighting slSpotADS;

    GLShader() {}
    ~GLShader() {}

public:
    static ShaderProgram *currShader;

    static void Load();
    static void Unload();
    static void Initialize();
    static void Terminate();
    static void Invalidate();

    static bool Start();
    static void Suspend();

    static void EnableShaders(xState val)
    {
        if (shaderState == xState_Disable && val != xState_Enable)
            return; // textures are disabled
        if (val == xState_Disable) Suspend();
        shaderState = val;
    }

    static bool NeedNormals()
    {
        return diffuse || specular;
    }

    static void SetLightType(xLightType type, bool ambient = true, bool diffuse = true, bool specular = true)
    {
        lightType = type;
        if (type == xLight_NONE)
        {
            glDisable(GL_LIGHTING);
            GLShader::ambient  = false;
            GLShader::diffuse  = false;
            GLShader::specular = false;
        }
        else
        {
            glEnable(GL_LIGHTING);
            GLShader::ambient  = ambient;
            GLShader::diffuse  = diffuse;
            GLShader::specular = specular;
        }
    }

    static xLightType GetLightType()
    {
        return lightType;
    }

    static void EnableTexturing(xState val)
    {
        if (textureState == xState_Disable && val != xState_Enable)
            return; // textures are disabled
        textureState = val;
        if (val == xState_On)
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);
    }

    static void EnableSkeleton(xState val)
    {
        if (skeletalState == xState_Disable && val != xState_Enable)
            return; // skeleton is disabled
        skeletalState = val;
    }

    static xState TexturingState()
    {
        return textureState;
    }

    static xState SkeletonState()
    {
        return skeletalState;
    }
};

#endif
