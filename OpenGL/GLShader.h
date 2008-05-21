#ifndef __incl_GLShader_h
#define __incl_GLShader_h     1

#include <cassert>
#include "../AppFramework/System.h"
#include <GL/gl.h>
#include "../GLExtensions/ARB_vertex_shader.h"
#include "../GLExtensions/ARB_fragment_shader.h"
#include "../Math/xMath.h"
#include "../Math/xLight.h"
#include "../Utils/Debug.h"

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

    static ShaderLighting slNoLighting;
    static ShaderLighting slGlobalAmbient;
    static ShaderLighting slInfiniteAmbient;
    static ShaderLighting slInfiniteDiffuseSpecular;
    static ShaderLighting slPointAmbient;
    static ShaderLighting slPointDiffuseSpecular;
    static ShaderLighting slSpotAmbient;
    static ShaderLighting slSpotDiffuseSpecular;

    GLShader() {}
    ~GLShader() {}

public:
    static ShaderProgram *currShader;

    static bool ambient;
    static bool diffuse;
    static bool specular;

    static void Load();
    static void Initialize();
    static void Terminate();
    static void Invalidate();

    static bool Start();
    static void Suspend()
    {
        //assert(program);
        //assert(IsCurrent());
        // Use The Fixed Function OpenGL
        if (shaderState == xState_Disable) return;
        shaderState = xState_Enable;
        glUseProgramObjectARB(ShaderProgram::currProgram = 0);
        currShader = NULL;
    }

    static void EnableShaders(xState val)
    {
        if (shaderState == xState_Disable && val != xState_Enable)
            return; // textures are disabled
        shaderState = val;
    }

    static void SetLightType(xLightType type)
    {
        lightType = type;
        if (type == xLight_NONE)
            glDisable(GL_LIGHTING);
        else
            glEnable(GL_LIGHTING);
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
