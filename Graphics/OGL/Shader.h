#ifndef __incl_Graphics_OGL_Shader_h
#define __incl_Graphics_OGL_Shader_h

#include "ogl.h"
#include "../../Math/xMath.h"
#include "../../Math/xLight.h"

namespace Graphics { namespace OGL {

    struct ShaderProgram
    {
        static GLenum currProgram;

        GLenum program;
        bool   FL_invalid;

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

        GLenum IsCreated() { return program; }
        bool IsCurrent() { return program == currProgram; }

        virtual GLenum Create();
        virtual void   Destroy();
        virtual void   Invalidate()
        {
            program = vertex_shader = fragment_shader = 0; FL_invalid = true;
        }
    };

    struct ShaderSkeletal : public ShaderProgram
    {
        // identifiers of vertex uniforms and attributes
        int uQuats;
        int uRoots;
        int uTrans;
        int aBoneIdxWghts;

        ShaderSkeletal();
        virtual ~ShaderSkeletal() {}

        virtual GLenum Create();
    };

    struct ShaderLighting
    {
        ShaderProgram  Plain;
        ShaderProgram  Textured;
        ShaderSkeletal PlainSkeletal;
        ShaderSkeletal TexturedSkeletal;

        void Create();
        void Destroy();
        void Invalidate();
    };

    enum xState
    {
        xState_Disable = -2,
        xState_Enable  = -1,
        xState_Off     =  0,
        xState_On      =  1
    };

    class Shader
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

        Shader() {}
        ~Shader() {}

    public:
        static ShaderProgram *currShader;

        static void Load();
        static void Unload();
        static void CreateS();
        static void DestroyS();
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
                Shader::ambient  = false;
                Shader::diffuse  = false;
                Shader::specular = false;
            }
            else
            {
                glEnable(GL_LIGHTING);
                Shader::ambient  = ambient;
                Shader::diffuse  = diffuse;
                Shader::specular = specular;
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

} } // namespace Graphics::OGL

#endif
